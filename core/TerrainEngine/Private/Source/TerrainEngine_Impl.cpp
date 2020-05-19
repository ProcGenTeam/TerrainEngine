#include "TerrainEngine/Private/Header/TerrainEngine_Impl.h"
#include "TerrainEngine/Private/Header/PerlinNoise.h"
#include "TerrainEngine/Private/Header/HydraulicErosion.h"
//#include <bits/stdint-uintn.h>
//#include <cmath>
#include <cstdint>

CTerrainEngine_Impl::CTerrainEngine_Impl
(
    uint32_t uWaterLevel,
    uint32_t uWidth,
    uint32_t uHeight,
    int32_t iXOffset,
    int32_t iYOffset,
    float fGlobalScale,
    uint32_t uOverscan,
    uint32_t uFilterSize
) : 
    m_uWaterLevel(uWaterLevel),
    m_uOverScan(std::max(uOverscan, uFilterSize * 2)),
    m_uWidth(uWidth + uOverscan * 2),
    m_uHeight(uHeight + uOverscan * 2),
    m_iXOffset(iXOffset),
    m_iYOffset(iYOffset),
    m_fGlobalScale(fGlobalScale),
    m_bImmediateMode(false),
    m_uFilterSize(uFilterSize),
    m_uStackPointer(0)
{
    // Create Default Layer
    auto temp = std::make_shared<std::vector<FLOAT_TYPE>>(std::vector<FLOAT_TYPE>(m_uHeight * m_uWidth));
    temp->resize(m_uHeight * m_uWidth);
    m_vpvData.push_back(std::move(temp));

    Internal_Initialise();

    Internal_TrackMemoryLoad(m_uHeight * m_uWidth * sizeof(FLOAT_TYPE), EMemoryUseTypes::LayerMemory);
    
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
    m_fGlobalScale(fScale),
    m_bImmediateMode(false)
{
    this->m_uHeight = vWorld->size() / uWidth;
    
    Internal_Initialise();
    // This is unique ptr
    //this->m_pData = std::move(vWorld);
}

void CTerrainEngine_Impl::Internal_Initialise()
{
    auto totalMemTypes = static_cast<std::underlying_type<EMemoryUseTypes>::type>(EMemoryUseTypes::TOTAL_MEMORY_TYPES);
    m_vTotalMemoryUsed.resize(totalMemTypes);
    m_vPeakMemoryUsed.resize(totalMemTypes);
}

CTerrainEngine_Impl::~CTerrainEngine_Impl()
{

}

////// ////// //////
// Poll Current State
//
std::shared_ptr<std::vector<FLOAT_TYPE>> CTerrainEngine_Impl::GetView(uint32_t uLayerIndex)
{
    return m_vpvData[uLayerIndex];
}

uint64_t CTerrainEngine_Impl::GetPeakMemoryUse(EMemoryUseTypes eMemoryType)
{
    auto index = static_cast<std::underlying_type<EMemoryUseTypes>::type>(eMemoryType);
    return m_vPeakMemoryUsed[index];
}

std::vector<FOperation> CTerrainEngine_Impl::GetHistory()
{
    return m_vQueue;
}

void CTerrainEngine_Impl::SetHistory(std::vector<FOperation> &vHistory)
{
    m_vQueue = vHistory;
}

void CTerrainEngine_Impl::SetHistory(std::vector<FOperation> *vHistory)
{
    m_vQueue = *vHistory;
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
    for(; m_uStackPointer < m_vQueue.size(); ++m_uStackPointer)
    {
        Internal_LazyEvaluate(m_vQueue[m_uStackPointer]);
    }
}

////// ////// //////
// Generation
//
void CTerrainEngine_Impl::Erode(uint32_t uLayerIndex, uint32_t uSteps, uint32_t uFilterSize)
{
    // Memory
    Internal_TrackMemoryLoad(sizeof(FLOAT_TYPE) * m_uHeight * m_uWidth, EMemoryUseTypes::MethodMemory);
    Internal_TrackMemoryLoad(sizeof(glm::vec3) * m_uHeight * m_uWidth, EMemoryUseTypes::MethodMemory);

    // Memory use isn't overly useful here
    if(this->m_bImmediateMode)
    {
        Internal_Erode(uLayerIndex, uSteps, uFilterSize);
    }
    else
    {
        FOperation op{};
        op.OpType = EOperationTypes::Erode;
        op.u32Arg1 = uLayerIndex;
        op.u32Arg2 = uSteps;
        op.u32Arg3 = uFilterSize;
        m_vQueue.push_back(std::move(op));
    }

    Internal_TrackMemoryLoad(-sizeof(FLOAT_TYPE) * m_uHeight * m_uWidth, EMemoryUseTypes::MethodMemory);
    Internal_TrackMemoryLoad(-sizeof(glm::vec3) * m_uHeight * m_uWidth, EMemoryUseTypes::MethodMemory);
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
    // Update rolling memory use
    Internal_TrackMemoryLoad(sizeof(FLOAT_TYPE) * m_uWidth * m_uHeight, EMemoryUseTypes::LayerMemory);

    if(this->m_bImmediateMode)
    {
        Internal_CreateLayer();
    }
    else
    {
        FOperation op{};
        op.OpType = EOperationTypes::CreateLayer;
        m_vQueue.push_back(std::move(op));
    }

    return m_vpvData.size() - 1;
}

void CTerrainEngine_Impl::DestroyLayer(uint32_t uLayerIndex)
{
    // Total needs to drop
    Internal_TrackMemoryLoad(-sizeof(FLOAT_TYPE) * m_uWidth * m_uHeight, EMemoryUseTypes::LayerMemory);

    if(this->m_bImmediateMode)
    {
        Internal_DestroyLayer(uLayerIndex);
    }
    else
    {
        FOperation op{};
        op.OpType = EOperationTypes::DestroyLayer;
        op.u32Arg1 = uLayerIndex;
        m_vQueue.push_back(std::move(op));
    }
}

void CTerrainEngine_Impl::MixLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer)
{
    if(this->m_bImmediateMode)
    {
        Internal_MixLayers(uDstLayer, uSrcLayer, uOtherSrcLayer);
    }
    else
    {
        FOperation op{};
        op.OpType = EOperationTypes::MixLayers;
        op.u32Arg1 = uDstLayer;
        op.u32Arg2 = uSrcLayer;
        op.u32Arg3 = uOtherSrcLayer;
        m_vQueue.push_back(std::move(op));
    }
}

void CTerrainEngine_Impl::RegisterMixingFunction(LayerMixer fnMixingFunction)
{
    m_vMixingFunctions.push_back(fnMixingFunction);
}

// TODO - This way of passing functions should be updated
// Add a Register Function routine and use a numeric
void CTerrainEngine_Impl::MixLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer, uint32_t uMixingFunctionIndex)
{
    if(this->m_bImmediateMode)
    {
        Internal_MixLayersCustom(uDstLayer, uSrcLayer, uOtherSrcLayer, m_vMixingFunctions[uMixingFunctionIndex]);
    }
    else
    {
        FOperation op{};
        op.OpType = EOperationTypes::MixLayersCustom;
        op.u32Arg1 = uDstLayer;
        op.u32Arg2 = uSrcLayer;
        op.u32Arg3 = uOtherSrcLayer;
        op.u32Arg4 = uMixingFunctionIndex;
        m_vQueue.push_back(std::move(op));
    }
}

void CTerrainEngine_Impl::AddLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer)
{
    if(this->m_bImmediateMode)
    {
        Internal_AddLayers(uDstLayer, uSrcLayer, uOtherSrcLayer);
    }
    else
    {
        FOperation op{};
        op.OpType = EOperationTypes::AddLayers;
        op.u32Arg1 = uDstLayer;
        op.u32Arg2 = uSrcLayer;
        op.u32Arg3 = uOtherSrcLayer;
        m_vQueue.push_back(std::move(op));
    }
}

void CTerrainEngine_Impl::SubLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer)
{
    if(this->m_bImmediateMode)
    {
        Internal_SubLayers(uDstLayer, uSrcLayer, uOtherSrcLayer);
    }
    else
    {
        FOperation op{};
        op.OpType = EOperationTypes::SubLayers;
        op.u32Arg1 = uDstLayer;
        op.u32Arg2 = uSrcLayer;
        op.u32Arg3 = uOtherSrcLayer;
        m_vQueue.push_back(std::move(op));
    }
}

void CTerrainEngine_Impl::MulLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer)
{
    if(this->m_bImmediateMode)
    {
        Internal_MulLayers(uDstLayer, uSrcLayer, uOtherSrcLayer);
    }
    else
    {
        FOperation op{};
        op.OpType = EOperationTypes::MulLayers;
        op.u32Arg1 = uDstLayer;
        op.u32Arg2 = uSrcLayer;
        op.u32Arg3 = uOtherSrcLayer;
        m_vQueue.push_back(std::move(op));
    }
}

void CTerrainEngine_Impl::DivLayers(uint32_t uDstLayer, uint32_t uSrcLayer, uint32_t uOtherSrcLayer)
{
    if(this->m_bImmediateMode)
    {
        Internal_DivLayers(uDstLayer, uSrcLayer, uOtherSrcLayer);
    }
    else
    {
        FOperation op{};
        op.OpType = EOperationTypes::DivLayers;
        op.u32Arg1 = uDstLayer;
        op.u32Arg2 = uSrcLayer;
        op.u32Arg3 = uOtherSrcLayer;
        m_vQueue.push_back(std::move(op));
    }
}

void CTerrainEngine_Impl::AddLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar)
{
    if(this->m_bImmediateMode)
    {
        Internal_AddLayerScalar(uDstLayer, uSrcLayer, fScalar);
    }
    else
    {
        FOperation op{};
        op.OpType = EOperationTypes::AddLayerScalar;
        op.u32Arg1 = uDstLayer;
        op.u32Arg2 = uSrcLayer;
        op.u32Arg3 = *reinterpret_cast<uint32_t *>(&fScalar);
        m_vQueue.push_back(std::move(op));
    }
}

void CTerrainEngine_Impl::SubLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar)
{
    if(this->m_bImmediateMode)
    {
        Internal_SubLayerScalar(uDstLayer, uSrcLayer, fScalar);
    }
    else
    {
        FOperation op{};
        op.OpType = EOperationTypes::SubLayerScalar;
        op.u32Arg1 = uDstLayer;
        op.u32Arg2 = uSrcLayer;
        op.u32Arg3 = *reinterpret_cast<uint32_t *>(&fScalar);
        m_vQueue.push_back(std::move(op));
    }
}

void CTerrainEngine_Impl::MulLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar)
{
    if(this->m_bImmediateMode)
    {
        Internal_MulLayerScalar(uDstLayer, uSrcLayer, fScalar);
    }
    else
    {
        FOperation op{};
        op.OpType = EOperationTypes::MulLayerScalar;
        op.u32Arg1 = uDstLayer;
        op.u32Arg2 = uSrcLayer;
        op.u32Arg3 = *reinterpret_cast<uint32_t *>(&fScalar);
        m_vQueue.push_back(std::move(op));
    }
}

void CTerrainEngine_Impl::DivLayerScalar(uint32_t uDstLayer, uint32_t uSrcLayer, float fScalar)
{
    if(this->m_bImmediateMode)
    {
        Internal_DivLayerScalar(uDstLayer, uSrcLayer, fScalar);
    }
    else
    {
        FOperation op{};
        op.OpType = EOperationTypes::DivLayerScalar;
        op.u32Arg1 = uDstLayer;
        op.u32Arg2 = uSrcLayer;
        op.u32Arg3 = *reinterpret_cast<uint32_t *>(&fScalar);
        m_vQueue.push_back(std::move(op));
    }
}