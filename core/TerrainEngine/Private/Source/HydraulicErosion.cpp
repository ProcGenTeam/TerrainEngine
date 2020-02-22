#include <cstdint>
#include "TerrainEngine/Private/Header/HydraulicErosion.h"
#include <algorithm>
#include <cstring>
#include <iostream>

CHydraulicErosion::CHydraulicErosion
(
    uint32_t uOverscan,
    uint32_t uSeed
) :
    m_uOverscan(uOverscan),
    m_mtRandGen(std::mt19937_64(uSeed)),
    m_distNormal(std::normal_distribution<float>(1.0, 0.1))
{

}

CHydraulicErosion::~CHydraulicErosion()
{

}

glm::vec3 CHydraulicErosion::GradientSum(FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uX, uint32_t uY, uint32_t uWidth, uint16_t uFilterWidth)
{
    glm::vec3 refPixel = glm::vec3(uX, uY, pHeight[uY * uWidth + uX]);
    glm::vec3 upDir(0,0,1);
    float erosionSum = 0.f;
    float oneOverBoxCount = 1 / (5 * 5 - 1);

    for(int32_t i = -uFilterWidth; i <= uFilterWidth; ++i)
    {
        auto modY = uY + i;
        auto iIndex = modY * uWidth;

        for(int32_t j = -uFilterWidth; j <= uFilterWidth; ++j)
        {
            if(i == 0 && j == 0) { continue; }

            auto modX = uX + j;

            glm::vec3 cPixel = glm::vec3(modX, modY, pHeight[iIndex + modX]);

            // Gradient
            glm::vec3 cPixGrad = cPixel - refPixel;

            auto drctn = glm::dot(cPixGrad, upDir) * (this->m_distNormal(m_mtRandGen));
            //if(drctn > 0) {drctn = 0.f;}

            // Only use outflowing directions
            //std::cout << drctn << std::endl;

            erosionSum += drctn;// * oneOverBoxCount;            
        }
    }
    return glm::vec3(erosionSum);
}

void CHydraulicErosion::Erode(FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uHeight, uint32_t uWidth, float fScale)
{
    // We start at one, because the outer pixels are unusable
    // We also substract one from the end for the same reason

#pragma omp parallel for
    for(uint32_t y = 0; y < uHeight; ++y)
    {
        auto my = y * uWidth;
        if(y < m_uOverscan || y >= uHeight - m_uOverscan)
        {
            std::memcpy(&pOut[my], &pHeight[my], sizeof(uint32_t) * uWidth);
        }
        else
        {
            for(uint32_t x = 0; x < uWidth; ++x)
            {
                if(x < m_uOverscan || x >= uWidth - m_uOverscan)
                {
                    pOut[my + x] = pHeight[my + x];
                }
                else
                {                
                    auto CentrePixel = pHeight[my + x];

                    // We want all pixels around this
                    auto Gradient = GradientSum(pHeight, pOut, x, y, uWidth, 2);

                    auto ero = Gradient.x * fScale;

                    pOut[my + x] = CentrePixel + ero;
                }
            }
        }
    }
}