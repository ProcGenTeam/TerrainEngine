#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include "TerrainEngine/Public/Header/Operations.h"
#include "TerrainEngine/Private/Header/TerrainEngine_Impl.h"
#include "TaskEngine/Public/Header/Tasks.h"
#include <queue>

class CTaskEngine_Impl
{
    private:
        std::vector<std::unique_ptr<CTerrainEngine_Impl>> m_vpTerrainEngine;
        std::vector<FOperation> m_vHistory;
        //std::unique_ptr<CNetworkEngine_Impl> m_pNetworkEngine;
        std::queue<FTask> m_qTasks;

    protected:
    public:
        CTaskEngine_Impl(uint64_t uWidth, uint64_t uHeight, uint32_t uTileSize);
        virtual ~CTaskEngine_Impl();

        virtual void Render(std::vector<FOperation> &&vHistory);

};