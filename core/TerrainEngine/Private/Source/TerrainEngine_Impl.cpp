#include "TerrainEngine/Private/Header/TerrainEngine_Impl.h"
#include "TerrainEngine/Private/Header/PerlinNoise.h"
//#include <bits/stdint-uintn.h>
//#include <cmath>
#include <cstdint>
#include <iostream>

//#include "extern/glm/"

#define GetInner(x) ((*x.get()).data())

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
    m_fScale(fScale),
    m_bImmediateMode(false)
{
    // Create Default Layer
    auto temp = std::make_shared<std::vector<uint32_t>>(std::vector<uint32_t>(m_uHeight * m_uWidth));
    temp->resize(m_uHeight * m_uWidth);
    m_vpvData.push_back(std::move(temp));
    
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
    m_fScale(fScale),
    m_bImmediateMode(false)
{
    this->m_uHeight = vWorld->size() / uWidth;
    
    // This is unique ptr
    //this->m_pData = std::move(vWorld);
}


CTerrainEngine_Impl::~CTerrainEngine_Impl()
{

}

////// ////// //////
// Poll Current State
//
std::shared_ptr<std::vector<uint32_t>> CTerrainEngine_Impl::GetView()
{
    return m_vpvData[0];
}

////// ////// //////
// Switch Mode
//
void CTerrainEngine_Impl::EnableImmediateMode()
{
    this->m_bImmediateMode = true;
}

void CTerrainEngine_Impl::DisableImmediateMode()
{
    this->m_bImmediateMode = false;

    // TODO Commit everything NOW!
    this->Render();
}
void CTerrainEngine_Impl::Render()
{
    return;
}

uint32_t CTerrainEngine_Impl::Internal_IsIndexSafe(uint32_t uLayerIndex)
{
    return (uLayerIndex < this->m_vpvData.size());
}


////// ////// //////
// Private Generation
//
void CTerrainEngine_Impl::Internal_Erode(uint32_t uSteps)
{
    auto rVec = GetInner(this->m_vpvData[0]);

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

void CTerrainEngine_Impl::Internal_Perlin(uint32_t uLayerIndex)
{
    if(!this->Internal_IsIndexSafe(uLayerIndex))
    {
        return;
    }

    auto pGen = CPerlinNoiseGen(0);

    auto rVec = GetInner(this->m_vpvData[uLayerIndex]);
    for(uint32_t y = 0; y < m_uHeight; ++y)
    {
        auto my = y * m_uWidth;

        for(uint32_t x = 0; x < m_uWidth; ++x)
        {
            auto NormalVal = std::floor(pGen.Generate(x, my, 0.01) * UINT16_MAX);
            std::cout << NormalVal << std::endl;
            rVec[my + x] = static_cast<uint32_t>(NormalVal);
        }
    }
}

////// ////// //////
// Generation
//
void CTerrainEngine_Impl::Erode(uint32_t uSteps)
{
    if(this->m_bImmediateMode)
    {
        Internal_Erode(uSteps);
    }
    else
    {
        FOperation op{};
        op.OpType = EOperationTypes::Erode;
        op.u32Arg1 = uSteps;
        m_vQueue.push_back(std::move(op));
    }
}

void CTerrainEngine_Impl::Perlin(uint32_t uLayerIndex)
{
    if(this->m_bImmediateMode)
    {
        Internal_Perlin(uLayerIndex);
    }
    else
    {
        FOperation op{};
        op.OpType = EOperationTypes::Perlin;
        op.u32Arg1 = uLayerIndex;
        m_vQueue.push_back(std::move(op));
    }
}


////// ////// //////
// Layer Control
//
uint32_t CTerrainEngine_Impl::CreateLayer()
{
    auto temp = std::make_shared<std::vector<uint32_t>>(std::vector<uint32_t>(m_uHeight * m_uWidth));
    temp->resize(m_uHeight * m_uWidth);
    m_vpvData.push_back(std::move(temp));

    return m_vpvData.size() - 1;
}

void CTerrainEngine_Impl::DestroyLayer(uint32_t uLayerIndex)
{
    
}