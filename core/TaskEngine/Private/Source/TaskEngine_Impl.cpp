#include "TaskEngine/Private/Header/TaskEngine_Impl.h"
#include "TaskEngine/Public/Header/ErrorCode.h"
#include "TerrainEngine/Private/Header/TerrainEngine_Impl.h"

#include <iostream>
#include <memory>


CTaskEngine_Impl::CTaskEngine_Impl(uint64_t uWidth, uint64_t uHeight, uint32_t uTileSize)
{
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
    if(m_vpNetworkEngine.empty()) { return ETaskErrorCodes::NoEngines; }


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

ETaskErrorCodes CTaskEngine_Impl::Listen(uint16_t uPort)
{
    return ETaskErrorCodes::TOTAL_ERROR_CODES;
}

ETaskErrorCodes CTaskEngine_Impl::RegisterNode(std::string strHostname, uint16_t uPort, std::vector<char> vKey)
{
    auto temp = std::make_shared<CNetworkEngine_Impl>();
    m_vpNetworkEngine.push_back(temp);

    temp->RegisterNode(strHostname, uPort, vKey);

    return ETaskErrorCodes::NoError;
}

ETaskErrorCodes CTaskEngine_Impl::RegisterNode(std::string strHostname, uint16_t uPort)
{
    auto temp = std::make_shared<CNetworkEngine_Impl>();
    m_vpNetworkEngine.push_back(temp);

    temp->RegisterNode(strHostname, uPort);
    
    return ETaskErrorCodes::NoError;
}