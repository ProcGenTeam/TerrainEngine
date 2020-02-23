#include "TerrainEngine/Private/Header/TerrainEngine_Impl.h"
#include "TerrainEngine/Private/Header/PerlinNoise.h"
#include "TerrainEngine/Private/Header/HydraulicErosion.h"
//#include <bits/stdint-uintn.h>
//#include <cmath>
#include <cstdint>

#define GetInner(x) ((*x.get()).data())

void CTerrainEngine_Impl::Internal_TrackMemoryLoad(int64_t iMemoryChange, EMemoryUseTypes eMemoryType)
{
    if(eMemoryType != EMemoryUseTypes::AllMemory)
    {
        auto allMemIndex = static_cast<std::underlying_type<EMemoryUseTypes>::type>(EMemoryUseTypes::AllMemory);

        // Update vTotal
        m_vTotalMemoryUsed[allMemIndex] += iMemoryChange;

        // Set Peak to max of Total or Old Peak, whichever is higher
        m_vPeakMemoryUsed[allMemIndex] = std::max(m_vPeakMemoryUsed[allMemIndex], m_vTotalMemoryUsed[allMemIndex]);
    }

    // Update the non-allmemory component

    // Update total memory, and keep track of keep
    auto index = static_cast<std::underlying_type<EMemoryUseTypes>::type>(eMemoryType);
    m_vTotalMemoryUsed[index] += iMemoryChange;

    m_vPeakMemoryUsed[index] = std::max(m_vPeakMemoryUsed[index], m_vTotalMemoryUsed[index]);
}

uint32_t CTerrainEngine_Impl::Internal_IsIndexSafe(uint32_t uLayerIndex)
{
    return (uLayerIndex < this->m_vpvData.size());
}


////// ////// //////
// Private Generation
//
void CTerrainEngine_Impl::Internal_Erode(uint32_t uLayerIndex, uint32_t uSteps, uint32_t uFilterSize)
{
    uFilterSize = uFilterSize ? uFilterSize : m_uFilterSize;

    auto iVec = this->m_vpvData[uLayerIndex];
    auto oVec = std::make_shared<std::vector<FLOAT_TYPE>>(std::vector<FLOAT_TYPE>(m_uHeight * m_uWidth));
    oVec->resize(m_uHeight * m_uWidth);

    auto oVecData = GetInner(oVec);
    auto iVecData = GetInner(iVec);

    auto eroder = CHydraulicErosion(uFilterSize, 0);

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
    auto oneOverHeight = 1.0 / (m_uHeight - m_uOverScan);
    auto oneOverWidth = 1.0 / (m_uWidth - m_uOverScan);

#pragma omp parallel for
    for(int64_t y = 0; y < m_uHeight; ++y)
    {
        auto my = y * m_uWidth;
        auto fy = (y - m_uOverScan + m_iYOffset) * oneOverHeight * m_fGlobalScale;

        for(int64_t x = 0; x < m_uWidth; ++x)
        {
            auto fx = (x - m_uOverScan + m_iXOffset) * oneOverWidth * m_fGlobalScale;
            auto NormalVal = pGen.Generate(fx * 5 * fScale, fy * 5 * fScale, 0.01);

            rVec[my + x] = NormalVal;
        }
    }
}


uint32_t CTerrainEngine_Impl::Internal_CreateLayer()
{
    // Update rolling memory use
    Internal_TrackMemoryLoad(sizeof(FLOAT_TYPE) * m_uWidth * m_uHeight, EMemoryUseTypes::LayerMemory);

    auto temp = std::make_shared<std::vector<FLOAT_TYPE>>(std::vector<FLOAT_TYPE>(m_uHeight * m_uWidth));
    temp->resize(m_uHeight * m_uWidth);
    m_vpvData.push_back(std::move(temp));

    return m_vpvData.size() - 1;
}

void CTerrainEngine_Impl::Internal_DestroyLayer(uint32_t uLayerIndex)
{
    // Total needs to drop
    Internal_TrackMemoryLoad(-sizeof(FLOAT_TYPE) * m_uWidth * m_uHeight, EMemoryUseTypes::LayerMemory);
}

void CTerrainEngine_Impl::Internal_MixLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer)
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

void CTerrainEngine_Impl::Internal_MixLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer, LayerMixer fnMixingFunction)
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

void CTerrainEngine_Impl::Internal_AddLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer)
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

void CTerrainEngine_Impl::Internal_SubLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer)
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

void CTerrainEngine_Impl::Internal_MulLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer)
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

void CTerrainEngine_Impl::Internal_DivLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer)
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

void CTerrainEngine_Impl::Internal_AddLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar)
{
    auto dstVec = GetInner(this->m_vpvData[uDstLayer]);
    auto srcVec = GetInner(this->m_vpvData[uSrcLayer]);

    auto uNumItems = m_uHeight * m_uWidth;
    for(uint32_t x = 0; x < uNumItems; ++x)
    {
        dstVec[x] = srcVec[x] + fScalar;
    }
}

void CTerrainEngine_Impl::Internal_SubLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar)
{
    auto dstVec = GetInner(this->m_vpvData[uDstLayer]);
    auto srcVec = GetInner(this->m_vpvData[uSrcLayer]);
    
    auto uNumItems = m_uHeight * m_uWidth;
    for(uint32_t x = 0; x < uNumItems; ++x)
    {
        dstVec[x] = srcVec[x] - fScalar;
    }
}

void CTerrainEngine_Impl::Internal_MulLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar)
{
    auto dstVec = GetInner(this->m_vpvData[uDstLayer]);
    auto srcVec = GetInner(this->m_vpvData[uSrcLayer]);
    
    auto uNumItems = m_uHeight * m_uWidth;
    for(uint32_t x = 0; x < uNumItems; ++x)
    {
        dstVec[x] = srcVec[x] * fScalar;
    }
}

void CTerrainEngine_Impl::Internal_DivLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar)
{
    auto dstVec = GetInner(this->m_vpvData[uDstLayer]);
    auto srcVec = GetInner(this->m_vpvData[uSrcLayer]);

    auto uNumItems = m_uHeight * m_uWidth;
    for(uint32_t x = 0; x < uNumItems; ++x)
    {
        dstVec[x] = srcVec[x] / fScalar;
    }
}