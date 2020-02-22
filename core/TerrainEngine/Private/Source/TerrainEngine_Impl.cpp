#include "TerrainEngine/Private/Header/TerrainEngine_Impl.h"
#include "TerrainEngine/Private/Header/PerlinNoise.h"
#include "TerrainEngine/Private/Header/HydraulicErosion.h"
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
    float fScale,
    uint32_t uOverscan
) : 
    m_uWaterLevel(uWaterLevel),
    m_uOverScan(uOverscan),
    m_uWidth(uWidth + uOverscan * 2),
    m_uHeight(uHeight + uOverscan * 2),
    m_fScale(fScale),
    m_bImmediateMode(false)
{
    // Create Default Layer
    auto temp = std::make_shared<std::vector<FLOAT_TYPE>>(std::vector<FLOAT_TYPE>(m_uHeight * m_uWidth));
    temp->resize(m_uHeight * m_uWidth);
    m_vpvData.push_back(std::move(temp));
    
    //m_pData = std::shared_ptr<std::vector<uint32_t>>(std::vector<uint32_t>[m_uWidth * m_uHeight]);
}

CTerrainEngine_Impl::CTerrainEngine_Impl
(
    uint32_t uWaterLevel,
    std::unique_ptr<std::vector<FLOAT_TYPE>> vWorld,
    uint32_t uWidth,
    float fScale,
    uint32_t uOverscan
) : 
    m_uWaterLevel(uWaterLevel),
    m_uOverScan(uOverscan),
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
std::shared_ptr<std::vector<FLOAT_TYPE>> CTerrainEngine_Impl::GetView()
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
void CTerrainEngine_Impl::Internal_Erode(uint32_t uLayerIndex, uint32_t uSteps)
{
    auto iVec = this->m_vpvData[uLayerIndex];
    auto oVec = std::make_shared<std::vector<FLOAT_TYPE>>(std::vector<FLOAT_TYPE>(m_uHeight * m_uWidth));
    oVec->resize(m_uHeight * m_uWidth);

    auto oVecData = GetInner(oVec);
    auto iVecData = GetInner(iVec);

    auto eroder = CHydraulicErosion(m_uOverScan, 0);


    // Buffer Rotations
    FLOAT_TYPE* l_pBuffers[2] = {iVecData, oVecData};
    std::shared_ptr<std::vector<FLOAT_TYPE>> l_pBufferPtrs[2] = {iVec, oVec};
    uint8_t uBufferIndex = 0;

    for(uint32_t step = 0; step < uSteps; ++step)
    {
        eroder.Erode(
            l_pBuffers[uBufferIndex],
            l_pBuffers[!uBufferIndex],
            m_uHeight,
            m_uWidth,
            0.1f
        );

        // Flip the buffer
        uBufferIndex = !uBufferIndex;
    }

    this->m_vpvData[uLayerIndex] = l_pBufferPtrs[uBufferIndex];
}

void CTerrainEngine_Impl::Internal_Perlin(uint32_t uLayerIndex, float fScale)
{
    if(!this->Internal_IsIndexSafe(uLayerIndex))
    {
        return;
    }

    auto pGen = CPerlinNoiseGen(0);

    auto rVec = GetInner(this->m_vpvData[uLayerIndex]);
    auto oneOverHeight = 1.0 / m_uHeight;
    auto oneOverWidth = 1.0 / m_uWidth;


    for(uint32_t y = 0; y < m_uHeight; ++y)
    {
        auto my = y * m_uWidth;
        auto fy = y * oneOverHeight;

        for(uint32_t x = 0; x < m_uWidth; ++x)
        {
            auto fx = x * oneOverWidth;
            auto NormalVal = pGen.Generate(fx * 5 * fScale, fy * 5 * fScale, 0.01);

            rVec[my + x] = NormalVal;
        }
    }
}

////// ////// //////
// Generation
//
void CTerrainEngine_Impl::Erode(uint32_t uLayerIndex, uint32_t uSteps)
{
    if(this->m_bImmediateMode)
    {
        Internal_Erode(uLayerIndex, uSteps);
    }
    else
    {
        FOperation op{};
        op.OpType = EOperationTypes::Erode;
        op.u32Arg1 = uSteps;
        m_vQueue.push_back(std::move(op));
    }
}

void CTerrainEngine_Impl::Perlin(uint32_t uLayerIndex, float fScale)
{
    if(this->m_bImmediateMode)
    {
        Internal_Perlin(uLayerIndex, fScale);
    }
    else
    {
        FOperation op{};
        op.OpType = EOperationTypes::Perlin;
        op.u32Arg1 = uLayerIndex;
        op.u32Arg2 = *reinterpret_cast<uint32_t *>(&fScale);
        m_vQueue.push_back(std::move(op));
    }
}


////// ////// //////
// Layer Control
//
uint32_t CTerrainEngine_Impl::CreateLayer()
{
    auto temp = std::make_shared<std::vector<FLOAT_TYPE>>(std::vector<FLOAT_TYPE>(m_uHeight * m_uWidth));
    temp->resize(m_uHeight * m_uWidth);
    m_vpvData.push_back(std::move(temp));

    return m_vpvData.size() - 1;
}

void CTerrainEngine_Impl::DestroyLayer(uint32_t uLayerIndex)
{
    
}

void CTerrainEngine_Impl::MixLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer)
{
    auto dstVec = GetInner(this->m_vpvData[uDstLayer]);
    auto srcVec = GetInner(this->m_vpvData[uSrcLayer]);
    auto otsrcVec = GetInner(this->m_vpvData[uOtherSrcLayer]);


    auto uNumItems = m_uHeight * m_uWidth;
    for(uint32_t x = 0; x < uNumItems; ++x)
    {
        float sum = srcVec[x];
        sum += otsrcVec[x];

        dstVec[x] = sum * 0.5f;
    }
}

void CTerrainEngine_Impl::MixLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer, LayerMixer fnMixingFunction)
{
    auto dstVec = GetInner(this->m_vpvData[uDstLayer]);
    auto srcVec = GetInner(this->m_vpvData[uSrcLayer]);
    auto otsrcVec = GetInner(this->m_vpvData[uOtherSrcLayer]);


    auto uNumItems = m_uHeight * m_uWidth;
    for(uint32_t x = 0; x < uNumItems; ++x)
    {
        dstVec[x] = fnMixingFunction(srcVec[x], otsrcVec[x]);
    }
}

void CTerrainEngine_Impl::AddLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer)
{
    auto dstVec = GetInner(this->m_vpvData[uDstLayer]);
    auto srcVec = GetInner(this->m_vpvData[uSrcLayer]);
    auto otsrcVec = GetInner(this->m_vpvData[uOtherSrcLayer]);


    auto uNumItems = m_uHeight * m_uWidth;
    for(uint32_t x = 0; x < uNumItems; ++x)
    {
        dstVec[x] = srcVec[x] + otsrcVec[x];
    }
}

void CTerrainEngine_Impl::SubLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer)
{
    auto dstVec = GetInner(this->m_vpvData[uDstLayer]);
    auto srcVec = GetInner(this->m_vpvData[uSrcLayer]);
    auto otsrcVec = GetInner(this->m_vpvData[uOtherSrcLayer]);


    auto uNumItems = m_uHeight * m_uWidth;
    for(uint32_t x = 0; x < uNumItems; ++x)
    {
        dstVec[x] = srcVec[x] - otsrcVec[x];
    }
}

void CTerrainEngine_Impl::MulLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer)
{
    auto dstVec = GetInner(this->m_vpvData[uDstLayer]);
    auto srcVec = GetInner(this->m_vpvData[uSrcLayer]);
    auto otsrcVec = GetInner(this->m_vpvData[uOtherSrcLayer]);


    auto uNumItems = m_uHeight * m_uWidth;
    for(uint32_t x = 0; x < uNumItems; ++x)
    {
        dstVec[x] = srcVec[x] * otsrcVec[x];
    }
}

void CTerrainEngine_Impl::DivLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer)
{
    auto dstVec = GetInner(this->m_vpvData[uDstLayer]);
    auto srcVec = GetInner(this->m_vpvData[uSrcLayer]);
    auto otsrcVec = GetInner(this->m_vpvData[uOtherSrcLayer]);


    auto uNumItems = m_uHeight * m_uWidth;
    for(uint32_t x = 0; x < uNumItems; ++x)
    {
        dstVec[x] = srcVec[x] / otsrcVec[x];
    }
}

void CTerrainEngine_Impl::AddLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar)
{
    auto dstVec = GetInner(this->m_vpvData[uDstLayer]);
    auto srcVec = GetInner(this->m_vpvData[uSrcLayer]);

    auto uNumItems = m_uHeight * m_uWidth;
    for(uint32_t x = 0; x < uNumItems; ++x)
    {
        dstVec[x] = srcVec[x] + fScalar;
    }
}

void CTerrainEngine_Impl::SubLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar)
{
    auto dstVec = GetInner(this->m_vpvData[uDstLayer]);
    auto srcVec = GetInner(this->m_vpvData[uSrcLayer]);
    
    auto uNumItems = m_uHeight * m_uWidth;
    for(uint32_t x = 0; x < uNumItems; ++x)
    {
        dstVec[x] = srcVec[x] - fScalar;
    }
}

void CTerrainEngine_Impl::MulLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar)
{
    auto dstVec = GetInner(this->m_vpvData[uDstLayer]);
    auto srcVec = GetInner(this->m_vpvData[uSrcLayer]);
    
    auto uNumItems = m_uHeight * m_uWidth;
    for(uint32_t x = 0; x < uNumItems; ++x)
    {
        dstVec[x] = srcVec[x] * fScalar;
    }
}

void CTerrainEngine_Impl::DivLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar)
{
    auto dstVec = GetInner(this->m_vpvData[uDstLayer]);
    auto srcVec = GetInner(this->m_vpvData[uSrcLayer]);

    auto uNumItems = m_uHeight * m_uWidth;
    for(uint32_t x = 0; x < uNumItems; ++x)
    {
        dstVec[x] = srcVec[x] / fScalar;
    }
}