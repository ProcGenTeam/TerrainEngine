#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include "TerrainEngine/Public/Header/Operations.h"
#include "TerrainEngine/Public/Header/Defines.h"

class CTerrainEngine_Impl
{
    private:
        uint32_t m_uWaterLevel;
        uint32_t m_uWidth;
        uint32_t m_uHeight;
        float m_fScale;
        uint32_t m_uOverScan;

        std::vector<FOperation> m_vQueue;
        bool m_bImmediateMode;

        // We could spherically ray march but that would be expensive as the generation cost of the SDF is high without optimisation 

        // The data pointer, we own this
        // Let's make it smart!
        
        //uint32_t *m_pData;
        std::vector<std::shared_ptr<std::vector<FLOAT_TYPE>>> m_vpvData;

    protected:
        ////// ////// //////
        // Safety
        //
        virtual uint32_t Internal_IsIndexSafe(uint32_t uLayerIndex);

        ////// ////// //////
        // Private Generation
        //
        virtual void Internal_Erode(uint32_t uLayerIndex, uint32_t uSteps);
        virtual void Internal_Perlin(uint32_t uLayerIndex, float fScale);

    public:
        CTerrainEngine_Impl(uint32_t uWaterLevel, uint32_t uWidth, uint32_t uHeight, float fScale = 0.001f, uint32_t uOverscan = 2);
        CTerrainEngine_Impl(uint32_t uWaterLevel, std::unique_ptr<std::vector<FLOAT_TYPE>> vWorld, uint32_t uWidth, float fScale = 0.001f, uint32_t uOverscan = 2);
        virtual ~CTerrainEngine_Impl();

        ////// ////// //////
        // Poll Current State
        // 
        virtual std::shared_ptr<std::vector<FLOAT_TYPE>> GetView();

        ////// ////// //////
        // Switch Mode
        //
        virtual void EnableImmediateMode();
        virtual void DisableImmediateMode();
        virtual void Render();

        ////// ////// //////
        // Generation
        //
        virtual void Erode(uint32_t uLayerIndex, uint32_t uSteps);
        virtual void Perlin(uint32_t uLayerIndex, float fScale);

        ////// ////// //////
        // Layer Control
        //
        virtual uint32_t CreateLayer();
        virtual void DestroyLayer(uint32_t uLayerIndex);
        
        ////// ////// //////
        // Layer Combination
        //
        virtual void MixLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer);
        virtual void MixLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer, LayerMixer lmMixingFunction);


        virtual void AddLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer);
        virtual void SubLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer);
        virtual void MulLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer);
        virtual void DivLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer);

        virtual void AddLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar);
        virtual void SubLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar);
        virtual void MulLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar);
        virtual void DivLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar);

};