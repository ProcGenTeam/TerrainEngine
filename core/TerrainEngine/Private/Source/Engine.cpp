#include "TerrainEngine/Public/Header/Engine.h"
#include "TerrainEngine/Private/Header/Engine_Impl.h"

#define Super (reinterpret_cast<CTerrainEngine_Impl*>(this->m_implementation))

TerrainEngine::CTerrainEngine::CTerrainEngine(uint32_t uWaterLevel, uint32_t uWidth, uint32_t uHeight, float fScale)
 : m_implementation(new CTerrainEngine_Impl(uWaterLevel, uWidth, uHeight, fScale))
{

}

TerrainEngine::CTerrainEngine::~CTerrainEngine()
{
    delete Super;
}

std::shared_ptr<std::vector<uint32_t>> TerrainEngine::CTerrainEngine::GetView()
{
    return Super->GetView();
}

void TerrainEngine::CTerrainEngine::Erode(uint32_t uSteps)
{
    Super->Erode(uSteps);
}