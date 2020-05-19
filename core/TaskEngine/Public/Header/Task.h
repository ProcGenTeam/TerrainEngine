#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include "TerrainEngine/Public/Header/Operations.h"

struct FTask
{
    uint32_t iXOffset;
    uint32_t iYOffset;
};

struct FDispatchTask
{
    uint32_t iXOffset;
    uint32_t iYOffset;
    uint32_t uTileSize;
    uint32_t uHistLength;

    std::shared_ptr<std::vector<FOperation>> pvHistory;
};