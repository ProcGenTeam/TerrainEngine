#include "TerrainEngine/Public/Header/TerrainEngine.h"
#include "TerrainEngine/Private/Header/TerrainEngine_Impl.h"

#define Super (reinterpret_cast<CTerrainEngine_Impl*>(this->m_implementation))

TerrainEngine::CTerrainEngine::CTerrainEngine(uint32_t uWaterLevel, uint32_t uWidth, uint32_t uHeight, int32_t iXOffset, int32_t iYOffset, float fScale, uint32_t uOverscan, uint32_t uFilterSize)
 : m_implementation(new CTerrainEngine_Impl(uWaterLevel, uWidth, uHeight, iXOffset, iYOffset, fScale, uOverscan, uFilterSize))
{

}

TerrainEngine::CTerrainEngine::~CTerrainEngine()
{
    delete Super;
}

////// ////// //////
// Poll Current State
// 
std::shared_ptr<std::vector<FLOAT_TYPE>> TerrainEngine::CTerrainEngine::GetView(uint32_t uLayerIndex)
{
    return Super->GetView(uLayerIndex);
}

std::vector<FOperation> TerrainEngine::CTerrainEngine::GetHistory()
{
    return std::move(Super->GetHistory());
}

////// ////// //////
// Switch Mode
//
void TerrainEngine::CTerrainEngine::EnableImmediateMode()
{
    Super->EnableImmediateMode();
}

void TerrainEngine::CTerrainEngine::DisableImmediateMode()
{
    Super->DisableImmediateMode();
}

void TerrainEngine::CTerrainEngine::Render()
{
    Super->Render();
}

////// ////// //////
// Generation
//
void TerrainEngine::CTerrainEngine::Erode(uint32_t uLayerIndex, uint32_t uSteps)
{
    Super->Erode(uLayerIndex, uSteps);
}

void TerrainEngine::CTerrainEngine::Perlin(uint32_t uLayerIndex, float fScale)
{
    Super->Perlin(uLayerIndex, fScale);
}

////// ////// //////
// Layer Control
//
uint32_t TerrainEngine::CTerrainEngine::CreateLayer()
{
    return Super->CreateLayer();
}

void TerrainEngine::CTerrainEngine::DestroyLayer(uint32_t uLayerIndex)
{
    Super->DestroyLayer(uLayerIndex);
}

void TerrainEngine::CTerrainEngine::MixLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer)
{
    Super->MixLayers(uDstLayer, uSrcLayer, uOtherSrcLayer);
}

void TerrainEngine::CTerrainEngine::MixLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer, uint32_t uMixingFunctionIndex)
{
    Super->MixLayers(uDstLayer, uSrcLayer, uOtherSrcLayer, uMixingFunctionIndex);
}

void TerrainEngine::CTerrainEngine::AddLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer)
{
    Super->AddLayers(uDstLayer, uSrcLayer, uOtherSrcLayer);
}

void TerrainEngine::CTerrainEngine::SubLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer)
{
    Super->SubLayers(uDstLayer, uSrcLayer, uOtherSrcLayer);
}

void TerrainEngine::CTerrainEngine::MulLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer)
{
    Super->MulLayers(uDstLayer, uSrcLayer, uOtherSrcLayer);
}

void TerrainEngine::CTerrainEngine::DivLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer)
{
    Super->DivLayers(uDstLayer, uSrcLayer, uOtherSrcLayer);
}

void TerrainEngine::CTerrainEngine::AddLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar)
{
    Super->AddLayers(uDstLayer, uSrcLayer, fScalar);
}

void TerrainEngine::CTerrainEngine::SubLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar)
{
    Super->SubLayers(uDstLayer, uSrcLayer, fScalar);
}

void TerrainEngine::CTerrainEngine::MulLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar)
{
    Super->MulLayers(uDstLayer, uSrcLayer, fScalar);
}

void TerrainEngine::CTerrainEngine::DivLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar)
{
    Super->DivLayers(uDstLayer, uSrcLayer, fScalar);
}