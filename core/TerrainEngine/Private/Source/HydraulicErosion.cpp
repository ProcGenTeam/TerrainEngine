#include <cstdint>
#include "TerrainEngine/Private/Header/HydraulicErosion.h"
#include <algorithm>
#include <cstring>
#include <iostream>

CHydraulicErosion::CHydraulicErosion
(
    int32_t iOverscan,
    uint32_t uSeed
) :
    m_iOverscan(iOverscan),
    m_mtRandGen(std::mt19937_64(uSeed)),
    m_distNormal(std::normal_distribution<float>(1.0, 0.1))
{
    
}

CHydraulicErosion::~CHydraulicErosion()
{

}

glm::vec3 CHydraulicErosion::GradientSum(FGradSumParams &stParams)
{
    glm::vec3 refPixel = glm::normalize(glm::vec3(
        stParams.uX * stParams.fOneOverWidth,
        stParams.uY * stParams.fOneOverHeight,
        stParams.pHeight[stParams.uY * stParams.uWidth + stParams.uX]
    ));

    auto pGradients = m_vv3GradientMap.data();
    //return pGradients[stParams.uY * stParams.uWidth + stParams.uX];

    glm::vec3 upDir(0,0,1);
    float erosionSum = 0.f;

    auto nBins = (m_iOverscan * 2 + 1);
    float oneOverBoxCount = 1 / (nBins * nBins - 1);

    for(int32_t i = -m_iOverscan; i <= m_iOverscan; ++i)
    {
        auto modY = stParams.uY + i;
        auto iIndex = modY * stParams.uWidth;

        for(int32_t j = -m_iOverscan; j <= m_iOverscan; ++j)
        {
            // Ignore ourselves
            if(i == 0 && j == 0) { continue; }

            auto modX = stParams.uX + j;

            glm::vec3 tPixel = glm::normalize(glm::vec3(
                modX * stParams.fOneOverWidth,
                modY * stParams.fOneOverHeight,
                stParams.pHeight[iIndex + modX]
            ));

            // Gradient
            glm::vec3 TpToCp = refPixel - tPixel;
            
            glm::vec3 TpGradient = pGradients[iIndex + modX];

            auto similarity = glm::dot(TpToCp, TpGradient) * (this->m_distNormal(m_mtRandGen));

            //auto drctn = glm::dot(cPixGrad, upDir) * (this->m_distNormal(m_mtRandGen));
            //if(drctn > 0) {drctn = 0.f;}

            // Only use outflowing directions
            //std::cout << drctn << std::endl;

            erosionSum += similarity;// * oneOverBoxCount;            
        }
    }
    return glm::vec3(erosionSum);
}

glm::vec3 CHydraulicErosion::ExitGradients(FGradSumParams &stParams)
{
    glm::vec3 refPixel = glm::vec3(
        stParams.uX * stParams.fOneOverWidth,
        stParams.uY * stParams.fOneOverWidth,
        stParams.pHeight[stParams.uY * stParams.uWidth + stParams.uX]
    );

    glm::vec3 upDir(0,0,1);
    glm::vec3 gradSum(0);

    auto nBins = (m_iOverscan * 2 + 1);
    float oneOverBoxCount = 1 / (nBins * nBins - 1);

    // BIG NOTE
    // This 

    for(int32_t i = -m_iOverscan; i <= m_iOverscan; ++i)
    {
        int32_t modY = stParams.uY + i;
        auto iIndex = modY * stParams.uWidth;

        for(int32_t j = -m_iOverscan; j <= m_iOverscan; ++j)
        {
            auto modX = stParams.uX + j;

            // Get this pixel
            auto tPixel = glm::vec3(
                modX * stParams.fOneOverWidth,
                modY * stParams.fOneOverHeight,
                stParams.pHeight[iIndex + modX]
            );

            auto grad = tPixel - refPixel; 

            grad *= (glm::dot(grad, upDir) < 0.0);

            gradSum += grad;
        }
    }

    return gradSum;
    return glm::normalize(gradSum);
}

void CHydraulicErosion::GenerateGradientMap(FLOAT_TYPE *pHeight, uint32_t uWidth, uint32_t uHeight)
{
    m_vv3GradientMap.resize(uWidth * uHeight);
    auto l_pData = m_vv3GradientMap.data();

    FGradSumParams stParamsDef{};
    stParamsDef.pHeight = pHeight;
    stParamsDef.uWidth = uWidth;
    stParamsDef.uHeight = uHeight;
    stParamsDef.fOneOverHeight = 1.0 / uHeight;
    stParamsDef.fOneOverWidth = 1.0 / uWidth;

    FGradSumParams stParams{};

#ifdef NDEBUG
#pragma omp parallel for private(stParams)
#endif
    for(uint32_t y = m_iOverscan; y < uHeight - m_iOverscan; ++y)
    {
        //FGradSumParams
        stParams = stParamsDef;
        auto my = y * uWidth;
        
        for(uint32_t x = m_iOverscan; x < uWidth - m_iOverscan; ++x)
        {
            stParams.uX = x;
            stParams.uY = y;

            l_pData[my + x] = ExitGradients(stParams);
        }
    }
}

void CHydraulicErosion::Erode(FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uHeight, uint32_t uWidth, float fScale)
{
    // We start at one, because the outer pixels are unusable
    // We also substract one from the end for the same reason

    GenerateGradientMap(pHeight, uWidth, uHeight);

    // Create Param Structure - Speed :)
    FGradSumParams stParamsDef{};
    stParamsDef.pHeight = pHeight;
    stParamsDef.pOut = pOut;
    stParamsDef.uWidth = uWidth;
    stParamsDef.uHeight = uHeight;
    stParamsDef.fOneOverHeight = 1.0 / uHeight;
    stParamsDef.fOneOverWidth = 1.0 / uWidth;

    FGradSumParams stParams{};

#ifdef NDEBUG
#pragma omp parallel for private(stParams)
#endif
    for(uint32_t y = 0; y < uHeight; ++y)
    {
        //FGradSumParams
        stParams = stParamsDef;

        auto my = y * uWidth;

        if(y < m_iOverscan * 2 || y >= uHeight - m_iOverscan * 2)
        {
            std::memcpy(&pOut[my], &pHeight[my], sizeof(uint32_t) * uWidth);
        }
        else
        {
            for(uint32_t x = 0; x < uWidth; ++x)
            {
                if(x < m_iOverscan * 2 || x >= uWidth - m_iOverscan * 2)
                {
                    pOut[my + x] = pHeight[my + x];
                }
                else
                {   
                    auto CentrePixel = pHeight[my + x];

                    // Fill the struct and call the function
                    stParams.uX = x;
                    stParams.uY = y;
                    auto Gradient = GradientSum(stParams);

                    auto ero = Gradient.x * fScale;

                    pOut[my + x] = CentrePixel + ero;
                }
            }
        }
    }
}

