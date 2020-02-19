#include "core/Private/engine/Engine_Impl.h"

CTerrainEngine_Impl::CTerrainEngine_Impl
(
    uint32_t uWaterLevel,
    uint32_t uWidth,
    uint32_t uHeight,
    float fScale
) : 
    m_uWaterLevel(uWaterLevel),
    m_uWidth(uWidth),
    m_uHeight(uHeight),
    m_fScale(fScale)
{
    m_pData = std::make_shared<std::vector<uint32_t>>(std::vector<uint32_t>(m_uHeight * m_uWidth));
    m_pData->resize(m_uHeight * m_uWidth);
    //m_pData = std::shared_ptr<std::vector<uint32_t>>(std::vector<uint32_t>[m_uWidth * m_uHeight]);
}

CTerrainEngine_Impl::~CTerrainEngine_Impl()
{

}

void CTerrainEngine_Impl::Erode(uint32_t uSteps)
{
    
}

std::shared_ptr<std::vector<uint32_t>> CTerrainEngine_Impl::GetView()
{
    return m_pData;
}




