#pragma once

#include "TerrainEngine/Public/Header/Defines.h"
#include <cstdint>
#include <vector> // For std::vector
#include <memory> // SmartPointers
#include "TerrainEngine/Public/Header/Operations.h"


TE_NS_OPEN

class TERRAIN_ENGINE_API CTerrainEngine
{
    private:
        void *m_implementation;
    public:
    // Methods
        CTerrainEngine(FLOAT_TYPE fWaterLevel, uint32_t uWidth, uint32_t uHeight, int32_t iXOffset = 0, int32_t iYOffset = 0, float fScale = 0.001f, uint32_t uOverscan = 2, uint32_t uFilterSize = 2);
        ~CTerrainEngine();

        ////// ////// //////
        // Poll Current State
        // 
        std::shared_ptr<std::vector<FLOAT_TYPE>> GetView(uint32_t uLayerIndex);
        std::vector<FOperation> GetHistory();

        ////// ////// //////
        // Switch Mode
        //
        void EnableImmediateMode();
        void DisableImmediateMode();
        void Render();

        ////// ////// //////
        // Generation
        //
        void Erode(uint32_t uLayerIndex, uint32_t uSteps, uint32_t uTerrainLayerIndex = UINT32_MAX, uint32_t uFilterSize = 0);
        void ErodeByNormals(uint32_t uLayerIndex, uint32_t uSteps);
        void Perlin(uint32_t uLayerIndex, float fScale);

        ////// ////// //////
        // Layer Control
        //
        uint32_t CreateLayer();
        void DestroyLayer(uint32_t uLayerIndex);

        void MixLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer);
        void MixLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer, uint32_t uMixingFunctionIndex);

        void AddLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer);
        void SubLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer);
        void MulLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer);
        void DivLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer);

        void AddLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar);
        void SubLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar);
        void MulLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar);
        void DivLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar);

};

TE_NS_CLOSE