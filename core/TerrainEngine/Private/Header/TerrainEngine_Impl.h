#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include "TerrainEngine/Public/Header/Operations.h"

class CTerrainEngine_Impl
{
    private:
        uint32_t m_uWaterLevel;
        uint32_t m_uWidth;
        uint32_t m_uHeight;
        float m_fScale;

        std::vector<FOperation> m_vQueue;

        // We could spherically ray march but that would be expensive as the generation cost of the SDF is high without optimisation 

        // The data pointer, we own this
        // Let's make it smart!
        
        //uint32_t *m_pData;
        std::shared_ptr<std::vector<uint32_t>> m_pData;

    protected:
    public:
        CTerrainEngine_Impl(uint32_t uWaterLevel, uint32_t uWidth, uint32_t uHeight, float fScale = 0.001f);
        CTerrainEngine_Impl(uint32_t uWaterLevel, std::unique_ptr<std::vector<uint32_t>> vWorld, uint32_t uWidth, float fScale = 0.001f);
        virtual ~CTerrainEngine_Impl();

        // Poll Current State
        // 
        virtual std::shared_ptr<std::vector<uint32_t>> GetView();

        virtual void Erode(uint32_t uSteps);
};