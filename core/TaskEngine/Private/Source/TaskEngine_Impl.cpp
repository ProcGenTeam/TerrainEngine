#include "TaskEngine/Private/Header/TaskEngine_Impl.h"
#include "TerrainEngine/Private/Header/TerrainEngine_Impl.h"


#include <iostream>

CTaskEngine_Impl::CTaskEngine_Impl(uint64_t uWidth, uint64_t uHeight, uint32_t uTileSize)
{
    auto uXTiles = ((uWidth - 1) / uTileSize) + 1;
    auto uYTiles = ((uHeight - 1) / uTileSize) + 1;

    std::cout << "Generating " << uXTiles * uYTiles << " tiles (" << uTileSize * uTileSize * uXTiles * uYTiles << " pixels)" << std::endl;

    // Generate Tasks
    for(uint32_t y = 0; y < uYTiles; ++y)
    {
        std::cout << y * uTileSize << " " << (y+1) * (uTileSize) - 1 << std::endl;
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

void CTaskEngine_Impl::Render(std::vector<FOperation> &&vHistory)
{
    m_vHistory = std::move(vHistory);


}