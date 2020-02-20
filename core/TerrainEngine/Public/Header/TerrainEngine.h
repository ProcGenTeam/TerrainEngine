#pragma once

#include "TerrainEngine/Public/Header/Defines.h"
#include <cstdint>
#include <vector> // For std::vector
#include <memory> // SmartPointers



TE_NS_OPEN

class TERRAIN_ENGINE_API CTerrainEngine
{
    private:
        void *m_implementation;
    public:
    // Methods
        CTerrainEngine(uint32_t uWaterLevel, uint32_t uWidth, uint32_t uHeight, float fScale = 0.001f);
        ~CTerrainEngine();

        // Get View to preview state
        // 
        std::shared_ptr<std::vector<uint32_t>> GetView();

        void Erode(uint32_t uSteps);
        void Perlin(uint32_t uLayer);
};

TE_NS_CLOSE