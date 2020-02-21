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
        bool m_bImmediateMode;

        // We could spherically ray march but that would be expensive as the generation cost of the SDF is high without optimisation 

        // The data pointer, we own this
        // Let's make it smart!
        
        //uint32_t *m_pData;
        std::vector<std::shared_ptr<std::vector<uint32_t>>> m_vpvData;

    protected:
        ////// ////// //////
        // Safety
        //
        virtual uint32_t Internal_IsIndexSafe(uint32_t uLayerIndex);

        ////// ////// //////
        // Private Generation
        //
        virtual void Internal_Erode(uint32_t uSteps);
        virtual void Internal_Perlin(uint32_t uLayerIndex);

    public:
        CTerrainEngine_Impl(uint32_t uWaterLevel, uint32_t uWidth, uint32_t uHeight, float fScale = 0.001f);
        CTerrainEngine_Impl(uint32_t uWaterLevel, std::unique_ptr<std::vector<uint32_t>> vWorld, uint32_t uWidth, float fScale = 0.001f);
        virtual ~CTerrainEngine_Impl();

        ////// ////// //////
        // Poll Current State
        // 
        virtual std::shared_ptr<std::vector<uint32_t>> GetView();

        ////// ////// //////
        // Switch Mode
        //
        virtual void EnableImmediateMode();
        virtual void DisableImmediateMode();
        virtual void Render();

        ////// ////// //////
        // Generation
        //
        virtual void Erode(uint32_t uSteps);
        virtual void Perlin(uint32_t uLayerIndex);

        ////// ////// //////
        // Layer Control
        //
        virtual uint32_t CreateLayer();
        virtual void DestroyLayer(uint32_t uLayerIndex);
};