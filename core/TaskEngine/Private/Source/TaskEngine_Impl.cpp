#include "TaskEngine/Private/Header/TaskEngine_Impl.h"
#include "TaskEngine/Public/Header/ErrorCode.h"
#include "TerrainEngine/Private/Header/TerrainEngine_Impl.h"

#include <iostream>
#include <memory>


CTaskEngine_Impl::CTaskEngine_Impl(uint64_t uWidth, uint64_t uHeight, uint32_t uTileSize)
: m_bShouldThreadTeardown(false),
    m_uTileSize(uTileSize)
{
    // Make the pool
    for(uint32_t i = 0; i < std::thread::hardware_concurrency(); ++i)
    {
        m_vtrTaskPool.push_back(std::thread(&CTaskEngine_Impl::RT_Process, this));
    }

    auto uXTiles = ((uWidth - 1) / uTileSize) + 1;
    auto uYTiles = ((uHeight - 1) / uTileSize) + 1;

    std::cout << "Generating " << uXTiles * uYTiles << " tiles (" << uTileSize * uTileSize * uXTiles * uYTiles << " pixels)" << std::endl;

    // Generate Tasks
    for(uint32_t y = 0; y < uYTiles; ++y)
    {
        for(uint32_t x = 0; x < uXTiles; ++x)
        {
            FTask task{};
            task.iXOffset = x * uTileSize;
            task.iYOffset = y * uTileSize;
            m_qTasks.emplace(std::move(task));
        }
    }
}

CTaskEngine_Impl::~CTaskEngine_Impl()
{
    
}

ETaskErrorCodes CTaskEngine_Impl::ScheduleTask(FTask &stTask, std::vector<FOperation> &vHistory)
{
    if(m_vpNetworkEngine.empty()) { return ETaskErrorCodes::Task_NoEngines; }


    // Pick best engine
    m_vpNetworkEngine[0]->SubmitTask(stTask, vHistory);

    return ETaskErrorCodes::NoError;
}

ETaskErrorCodes CTaskEngine_Impl::Render(std::vector<FOperation> &vHistory)
{
    while(m_qTasks.size() > 0)
    {
        auto task = m_qTasks.front();

        auto ret = ScheduleTask(task, vHistory);
        if(ret != ETaskErrorCodes::NoError) { return ret; }

        m_qTasks.pop();
    }

    return ETaskErrorCodes::NoError;

}

void CTaskEngine_Impl::RT_Process()
{
    while(true)
    {
        std::unique_lock<std::mutex> lock(m_mtxSignal);
        m_cvSignal.wait(lock, [this](){
            return !m_qProcTasks.empty();
        });

        auto t = m_qProcTasks.front();
        m_qProcTasks.pop();

        // Unlock and notify
        lock.unlock();
        m_cvSignal.notify_one();

        auto eng = CTerrainEngine_Impl(0, m_uTileSize, m_uTileSize, t.iXOffset, t.iYOffset);

        eng.SetHistory(t.pvHistory.get());

        auto mmu = eng.GetPeakMemoryUse();

        // Should we go into hold-off?
        // These should be functions!
        uint32_t systemMemoryUsed = 0;
        uint32_t systemTotalUsableMem = 100;

        if(mmu > systemTotalUsableMem)
        {
            // Ha...

            continue;
        }

        while(systemMemoryUsed + mmu > systemTotalUsableMem)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        eng.Render();
    }
}

void CTaskEngine_Impl::SubmitMessage(FMessage &&Message)
{
    std::unique_lock<std::mutex> lock(m_mtxMessageQueue);

    // We have no wait condition :)
    m_qMessageQueue.emplace(std::move(Message));

    // Unlock and notify
    lock.unlock();
    m_cvMessageQueue.notify_one();
}

FMessage CTaskEngine_Impl::ReadMessage()
{
    // Wait on not being empty
    std::unique_lock<std::mutex> lock(m_mtxMessageQueue);

    m_cvMessageQueue.wait(lock, [this](){
        return !m_qMessageQueue.empty();
    });

    // Get Data
    auto retVar = m_qMessageQueue.front();
    m_qMessageQueue.pop();

    // Unlock.
    // Not strictly necessary are unique_lock is scoped
    lock.unlock();

    return retVar;
}

void CTaskEngine_Impl::RT_Dispatch()
{
    while(true)
    {
        if(m_bShouldThreadTeardown) return;

        FDispatchTask lTask{};

        auto ret = m_pNetServer->GetRequest(lTask);
        if (ret != ETaskErrorCodes::NoError)
        {
            FMessage msg{};
            msg.ErrorType = ret;
            msg.ThreadType = EThreadType::Task_NetHandler;
            SubmitMessage(std::move(msg));
            continue;
        }

        // Otherwise
        std::unique_lock<std::mutex> lock(m_mtxSignal);

        m_qProcTasks.push(std::move(lTask));

        // Unlock and notify
        lock.unlock();
        m_cvSignal.notify_one();
    }
}

ETaskErrorCodes CTaskEngine_Impl::Listen(uint16_t uPort)
{
    m_pNetServer = std::make_shared<CNetworkEngine_Impl>();

    auto err = m_pNetServer->Listen(uPort);
    if (err != ETaskErrorCodes::NoError) { return err; }

    m_trServer = std::thread(&CTaskEngine_Impl::RT_Dispatch, this);

    return err;
}

ETaskErrorCodes CTaskEngine_Impl::RegisterNode(std::string strHostname, uint16_t uPort, std::vector<char> vKey)
{
    auto temp = std::make_shared<CNetworkEngine_Impl>();
    m_vpNetworkEngine.push_back(temp);

    auto ret = temp->RegisterNode(strHostname, uPort, vKey);

    return ret;
}

ETaskErrorCodes CTaskEngine_Impl::RegisterNode(std::string strHostname, uint16_t uPort)
{
    auto temp = std::make_shared<CNetworkEngine_Impl>();
    m_vpNetworkEngine.push_back(temp);

    auto ret = temp->RegisterNode(strHostname, uPort);
 
    return ret;
}