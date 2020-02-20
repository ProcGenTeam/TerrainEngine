#include "TerrainEngine/Private/Header/TerrainEngine_Impl.h"

#define GetInner(x) (&(*x.get())[0])

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

CTerrainEngine_Impl::CTerrainEngine_Impl
(
    uint32_t uWaterLevel,
    std::unique_ptr<std::vector<uint32_t>> vWorld,
    uint32_t uWidth,
    float fScale
) : 
    m_uWaterLevel(uWaterLevel),
    m_uWidth(uWidth),
    m_fScale(fScale)
{
    this->m_uHeight = vWorld->size() / uWidth;
    
    // This is unique ptr
    this->m_pData = std::move(vWorld);
}


CTerrainEngine_Impl::~CTerrainEngine_Impl()
{

}

void CTerrainEngine_Impl::Erode(uint32_t uSteps)
{
    FOperation op{};
    op.OpType = EOperationTypes::Erode;
    op.u32Arg1 = uSteps;

    auto rVec = GetInner(this->m_pData);

    m_vQueue.push_back(std::move(op));

    for(uint32_t i = 0; i < uSteps; ++i)
    {
        for(uint32_t y = 0; y < m_uHeight; ++y)
        {
            auto my = y * m_uWidth;

            for(uint32_t x = 0; x < m_uWidth; ++x)
            {
                rVec[my + x] = my+x;
            }
        }
    }
}

std::shared_ptr<std::vector<uint32_t>> CTerrainEngine_Impl::GetView()
{
    return m_pData;
}




