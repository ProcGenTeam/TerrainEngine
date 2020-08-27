#pragma once
#include "NetworkEngine/Private/Header/NetworkEngine_Impl.h"
#include "TaskEngine/Public/Header/ErrorCode.h"
#include "TaskEngine/Public/Header/Task.h"
#include "TerrainEngine/Private/Header/TerrainEngine_Impl.h"
#include "TerrainEngine/Public/Header/Operations.h"
#include <cstdint>
#include <memory>
#include <queue>
#include <vector>

class CTaskEngine_Impl
{
private:
    // std::vector<std::shared_ptr<CTerrainEngine_Impl>> m_vpTerrainEngine;
    std::vector<std::shared_ptr<CNetworkEngine_Impl>> m_vpNetworkEngine;

    std::shared_ptr<CNetworkEngine_Impl> m_pNetServer;

    uint32_t m_uTileSize;

    std::queue<FTask> m_qTasks;

    std::thread m_trServer;
    std::thread m_trNetwork;

    std::vector<std::thread> m_vtrTaskPool;
    std::condition_variable m_cvSignal;
    std::queue<FDispatchTask> m_qProcTasks;
    std::mutex m_mtxSignal;
    bool m_bShouldThreadTeardown;

    // Message
    std::condition_variable m_cvMessageQueue;
    std::mutex m_mtxMessageQueue;
    std::queue<FMessage> m_qMessageQueue;

protected:
private:
    virtual ETaskErrorCodes ScheduleTask(FTask &stTask, std::vector<FOperation> &vHistory);

    virtual void RT_Dispatch();
    virtual void RT_Process();

    virtual void SubmitMessage(FMessage &&Message);
    virtual FMessage ReadMessage();

public:
    CTaskEngine_Impl(uint64_t uWidth, uint64_t uHeight, uint32_t uTileSize);
    virtual ~CTaskEngine_Impl();

    virtual ETaskErrorCodes Render(std::vector<FOperation> &vHistory);

    virtual ETaskErrorCodes RegisterNode(std::string strHostname, uint16_t uPort, std::vector<char> vKey);
    virtual ETaskErrorCodes RegisterNode(std::string strHostname, uint16_t uPort);

    virtual ETaskErrorCodes Listen(uint16_t uPort);
};