#pragma once

#if defined(_MSC_VER)
    #ifdef __TERRAIN_ENGINE_EXPORTS__
        #define TERRAIN_ENGINE_API __declspec(dllexport)
    #else
        #define TERRAIN_ENGINE_API __declspec(dllimport)
    #endif
#elif defined(__GNUC__)
    #ifdef __TERRAIN_ENGINE_EXPORTS__
        #define TERRAIN_ENGINE_API __attribute__((visibility("default")))
    #else
        #define TERRAIN_ENGINE_API
    #endif
#else
    #define TERRAIN_ENGINE_API
#endif

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
};

TE_NS_CLOSE