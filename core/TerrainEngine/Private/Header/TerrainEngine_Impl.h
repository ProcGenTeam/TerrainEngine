#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include "Common/Public/Header/Types.h"
#include "TerrainEngine/Public/Header/Operations.h"
#include "TerrainEngine/Public/Header/Defines.h"
#include "TerrainEngine/Private/Header/HydraulicErosion.h"

class CTerrainEngine_Impl
{
    private:
        FLOAT_TYPE m_fWaterLevel;
        uint32_t m_uWidth;
        uint32_t m_uHeight;
        int32_t m_iXOffset;
        int32_t m_iYOffset;
        float m_fGlobalScale;
        uint32_t m_uOverScan;
        uint32_t m_uFilterSize;

        uint32_t m_uStackPointer;
        std::vector<FOperation> m_vQueue;
        bool m_bImmediateMode;


        std::vector<uint64_t> m_vTotalMemoryUsed;
        std::vector<uint64_t> m_vPeakMemoryUsed;

        std::vector<LayerMixer> m_vMixingFunctions;
        // We could spherically ray march but that would be expensive as the generation cost of the SDF is high without optimisation 

        // The data pointer, we own this
        // Let's make it smart!
        
        //uint32_t *m_pData;
        std::vector<std::shared_ptr<std::vector<FLOAT_TYPE>>> m_vpvData;
        
    private: 
        void Internal_Initialise();

    protected:
        ////// ////// //////
        //
        //
        virtual void Internal_LazyEvaluate(FOperation &stOp);

        ////// ////// //////
        // Safety
        //
        virtual uint32_t Internal_IsIndexSafe(uint32_t uLayerIndex);

        //////
        virtual void Internal_TrackMemoryLoad(int64_t iMemoryUse, EMemoryUseTypes eMemoryType);

        virtual CHydraulicErosion* Internal_GetBestEroder(uint32_t uFilterSize, uint32_t uSeed, int32_t iOffsetX, int32_t iOffsetY, FLOAT_TYPE fWaterLevel);

        ////// ////// //////
        // Private Generation
        //
        virtual void Internal_Erode(uint32_t uLayerIndex, uint32_t uSteps, uint32_t uTerrainLayerIndex, uint32_t uFilterSize);
        virtual void Internal_ErodeByNormals(uint32_t uLayerIndex, uint32_t uSteps);
        virtual void Internal_Perlin(uint32_t uLayerIndex, float fScale);

        ////// ////// //////
        // Private Layer Control
        // 
        virtual uint32_t Internal_CreateLayer();
        virtual void Internal_DestroyLayer(uint32_t uLayerIndex);
        
        ////// ////// //////
        // Private Layer Combination
        //
        virtual void Internal_MixLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer);
        virtual void Internal_MixLayersCustom(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer, LayerMixer lmMixingFunction);

        virtual void Internal_AddLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer);
        virtual void Internal_SubLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer);
        virtual void Internal_MulLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer);
        virtual void Internal_DivLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer);

        virtual void Internal_AddLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar);
        virtual void Internal_SubLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar);
        virtual void Internal_MulLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar);
        virtual void Internal_DivLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar);

    public:
        CTerrainEngine_Impl(FLOAT_TYPE fWaterLevel, uint32_t uWidth, uint32_t uHeight, int32_t iXOffset = 0, int32_t iYOffset = 0, float fScale = 0.001f, uint32_t uOverscan = 2, uint32_t uFilterSize = 2);
        CTerrainEngine_Impl(FLOAT_TYPE fWaterLevel, std::unique_ptr<std::vector<FLOAT_TYPE>> vWorld, uint32_t uWidth, float fScale = 0.001f, uint32_t uOverscan = 2);
        virtual ~CTerrainEngine_Impl();

        ////// ////// //////
        // Poll Current State
        // 
        virtual std::shared_ptr<std::vector<FLOAT_TYPE>> GetView(uint32_t uLayerIndex);
        virtual uint64_t GetPeakMemoryUse(EMemoryUseTypes eMemoryType = EMemoryUseTypes::AllMemory);
        virtual std::vector<FOperation> GetHistory();
        virtual void SetHistory(std::vector<FOperation> &vHistory);
        virtual void SetHistory(std::vector<FOperation> *vHistory);

        ////// ////// //////
        // Switch Mode
        //
        virtual void EnableImmediateMode();
        virtual void DisableImmediateMode();
        virtual void Render();

        ////// ////// //////
        // Generation
        //
        virtual void Erode(uint32_t uLayerIndex, uint32_t uSteps, uint32_t uTerrainLayerIndex = UINT32_MAX, uint32_t uFilterSize = 0);
        virtual void ErodeByNormals(uint32_t uLayerIndex, uint32_t uSteps);
        virtual void Perlin(uint32_t uLayerIndex, float fScale);

        ////// ////// //////
        // Layer Control
        //
        virtual uint32_t CreateLayer();
        virtual void DestroyLayer(uint32_t uLayerIndex);

        virtual void RegisterMixingFunction(LayerMixer fnMixingFunction);
        
        ////// ////// //////
        // Layer Combination
        //
        virtual void MixLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer);
        virtual void MixLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer, uint32_t uMixingFunctionIndex);

        virtual void AddLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer);
        virtual void SubLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer);
        virtual void MulLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer);
        virtual void DivLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer);

        virtual void AddLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar);
        virtual void SubLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar);
        virtual void MulLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar);
        virtual void DivLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar);

};