#include "core/Public/engine/Engine.h"
#include "core/Private/engine/Engine_Impl.h"

#define Super (reinterpret_cast<CTerrainEngine_Impl*>(this->m_implementation))

TerrainEngine::CTerrainEngine::CTerrainEngine(uint32_t uWaterLevel, uint32_t uWidth, uint32_t uHeight, float fScale)
 : m_implementation(new CTerrainEngine_Impl(uWaterLevel, uWidth, uHeight, fScale))
{

}
