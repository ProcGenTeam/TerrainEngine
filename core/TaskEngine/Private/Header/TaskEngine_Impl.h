#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include "TerrainEngine/Public/Header/Operations.h"
#include "TerrainEngine/Private/Header/TerrainEngine_Impl.h"
#include "NetworkEngine/Private/Header/NetworkEngine_Impl.h"
#include "TaskEngine/Public/Header/Task.h"
#include "TaskEngine/Public/Header/ErrorCode.h"
#include <queue>

class CTaskEngine_Impl
{
    private:
        std::vector<std::shared_ptr<CTerrainEngine_Impl>> m_vpTerrainEngine;
        std::vector<std::shared_ptr<CNetworkEngine_Impl>> m_vpNetworkEngine;
        std::queue<FTask> m_qTasks;

    protected:


    private:
        virtual ETaskErrorCodes ScheduleTask(FTask &stTask, std::vector<FOperation> &vHistory);
    public:
        CTaskEngine_Impl(uint64_t uWidth, uint64_t uHeight, uint32_t uTileSize);
        virtual ~CTaskEngine_Impl();

        virtual ETaskErrorCodes Render(std::vector<FOperation> &vHistory);

        virtual ETaskErrorCodes RegisterNode(std::string strHostname, uint16_t uPort, std::vector<char> vKey);
        virtual ETaskErrorCodes RegisterNode(std::string strHostname, uint16_t uPort);

        virtual ETaskErrorCodes Listen(uint16_t uPort);
};