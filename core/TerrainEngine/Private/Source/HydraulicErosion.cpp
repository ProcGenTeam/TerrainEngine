#include <cfloat>
#include <cstdint>
#include "Common/Public/Header/Types.h"
#include "TerrainEngine/Private/Header/HydraulicErosion.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <omp.h>
//#include <math>

#define GetScaledHeight(x) (pHeight[x] * m_fWorldVerticalScale)
#define GetScaled(x) ((x) * m_fWorldVerticalScale)
#define GetWorldX(x) ((x) + m_iOffsetX)
#define GetWorldY(y) ((y) + m_iOffsetY)
#define GetWorldXY(x, width) ( (x) + (m_iOffsetY * (width) + m_iOffsetX) )

CHydraulicErosion::CHydraulicErosion
(
    int32_t iOverscan,
    uint32_t uSeed,
    int32_t iOffsetX,
    int32_t iOffsetY,
    FLOAT_TYPE fWaterLevel
) :
    m_iOverscan(iOverscan),
    m_iOffsetX(iOffsetX),
    m_iOffsetY(iOffsetY),
    m_mtRandGen(std::mt19937_64(uSeed)),
    m_distNormal(std::normal_distribution<float>(1.0, 0.1)),
    m_fWaterLevel(fWaterLevel),
    m_fDeposition(0.01f),
    m_fEvaporation(0.002f),
    m_fSedimentCapacity(2.f),
    m_fSoilSoftness(0.05f),
    m_fWorldVerticalScale(4000)
{
    
}

CHydraulicErosion::~CHydraulicErosion()
{

}

glm::vec3 CHydraulicErosion::GradientSum(FGradSumParams &stParams)
{
    glm::vec3 refPixel = glm::normalize(glm::vec3(
        stParams.uX * stParams.fOneOverWidth,
        stParams.pHeight[stParams.uY * stParams.uWidth + stParams.uX] * 255,
        stParams.uY * stParams.fOneOverHeight
    ));

    auto pGradients = m_vv3GradientMap.data();
    //return pGradients[stParams.uY * stParams.uWidth + stParams.uX];

    glm::vec3 upDir(0,1,0);
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
                stParams.pHeight[iIndex + modX] * 255,
                modY * stParams.fOneOverHeight
            ));

            // Gradient
            glm::vec3 TpToCp = glm::normalize(tPixel - refPixel);
            
            // glm::vec3 TpGradient = pGradients[iIndex + modX];
            // auto similarity = glm::dot(TpToCp, TpGradient) * (this->m_distNormal(m_mtRandGen));
            
            auto rawAngle = glm::dot(upDir, TpToCp);
            // Similarity to upwards angle. 1 is up, -1 is down
            auto similarity = acos(-rawAngle) * (2.f/3.1415926535) - 1; 
            
            erosionSum += similarity;// * fabs(tPixel.y - refPixel.y);// * (this->m_distNormal(m_mtRandGen));// * oneOverBoxCount; 


            //auto drctn = glm::dot(cPixGrad, upDir) * (this->m_distNormal(m_mtRandGen));
            //if(drctn > 0) {drctn = 0.f;}

            // Only use outflowing directions
            //std::cout << drctn << std::endl;           
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

    glm::vec3 upDir(0,1,0);
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
                stParams.pHeight[iIndex + modX] * 255,
                modY * stParams.fOneOverHeight
            );

            auto grad = tPixel - refPixel; 

            grad *= (glm::dot(grad, upDir) < 0.0);

            gradSum += grad;
        }
    }

    return gradSum;
    //return glm::normalize(gradSum);
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

void CHydraulicErosion::ErodeByNormals(FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uHeight, uint32_t uWidth, float fScale)
{
#ifdef NDEBUG
#pragma omp parallel for
#endif
    for(uint32_t y = 0; y < uHeight; ++y)
    {
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

                    auto rT = pHeight[(y+1) * uWidth + x] * 255;
                    auto rB = pHeight[(y-1) * uWidth + x] * 255;
                    auto rR = pHeight[(y) * uWidth + (x+1)] * 255;
                    auto rL = pHeight[(y) * uWidth + (x-1)] * 255;

                    auto pointNorm = glm::normalize(glm::vec3(2 * (rL-rR), 4, 2 * (rB-rT)));
                    auto rawAngle = glm::dot(pointNorm, glm::vec3(0,1,0));
                    auto asDeg = acos(rawAngle) * (2.f/3.1415926535);
                    auto asRemap = 1 - (fabs(0.5f - asDeg) * 2.f);
                    auto mulPt = (1 - 0.5 * glm::dot(pointNorm, glm::vec3(0,1,0)));
                    
                    pOut[my + x] = CentrePixel - asRemap * fScale;
                }
            }
        }
    }
}

void CHydraulicErosion::TestFunc(FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uHeight, uint32_t uWidth, float fScale)
{
    // Create Array. Let's use a *very* small step size to get some good performance
#ifdef NDEBUG
    uint32_t uSteps = 65536;
#else
    uint32_t uSteps = 1024;
#endif
    uSteps = 5000;
    float fStep = 1.f / uSteps;

    // Resize Arrays
    m_vv4WaterMap.resize(uHeight * uWidth);
    m_vstWaterCombiner.resize(uHeight * uWidth);
    m_vstSedimentCombiner.resize(uHeight * uWidth);

    // Set the Buffer to zeros
    memset(m_vv4WaterMap.data(), 0, m_vv4WaterMap.size() * sizeof(glm::vec4));
    memset(m_vstWaterCombiner.data(), 0, m_vstWaterCombiner.size() * sizeof(FHydFloatCombiner));
    memset(m_vstSedimentCombiner.data(), 0, m_vstSedimentCombiner.size() * sizeof(FHydFloatCombiner));

    // Set the WaterMap x's to 0.5 + the height of the terrain
    // y's to zero. There is no sediment yet

    #ifdef NDEBUG
    #pragma omp parallel for
    #endif
    for (uint32_t i = 0; i < m_vv4WaterMap.size(); ++i)
    {
        float rngMod = 1 + 0.001 * (double(uint64_t(6364136223846793005 * GetWorldXY(i, uWidth) + 1442695040888963407)) / double(UINT64_MAX));
        m_vv4WaterMap[i].x = 0.000125f * 0.5f + (pHeight[i] * rngMod * 0.000125f);
        m_vv4WaterMap[i].y = 0.f;

        //printf("Check Water Level at %d is %f vs %f\n", i,  pHeight[i], m_fWaterLevel);

        if(pHeight[i] < m_fWaterLevel)
        {
            //printf("Setting Water Level at %d to %f\n", i, m_fWaterLevel - pHeight[i]);
            m_vv4WaterMap[i].x = m_fWaterLevel - pHeight[i];
        }
    }

    //printf("Finished Creating Water\n");
    bool bNoWater = false;
    float fTotalWaterRemaining = 0;
    // Process Each Step
    for(uint32_t s = 0; s < uSteps; ++s)
    {
        if(s % 500 == 0){printf("Stepping %d\n",s);}

        float fStepWaterRemaining = 0;
        for (uint32_t i = 0; i < m_vv4WaterMap.size(); ++i)
        {
           fStepWaterRemaining += m_vv4WaterMap[i].x;
        }

        if(fStepWaterRemaining < (uWidth * uHeight * 0.00001))
        {
            bNoWater = true;
        }

        // if (fStepWaterRemaining > fTotalWaterRemaining)
        // {
        //     bRainIsExploding = true;
        // }
        // fTotalWaterRemaining = fStepWaterRemaining;
        // //if(s % 499 == 0){printf("Pre Rain Water: %f\n",fTotalWaterRemaining);}
        // if (bRainIsExploding)
        // {
        //     printf("Too much water caught! Holding off\n");
        // }


        if(bNoWater && s != 0 && s % 2 == 0 && s < (uSteps * 0.9))
        {

            #ifdef NDEBUG
            #pragma omp parallel for
            #endif
            for (uint32_t i = 0; i < m_vv4WaterMap.size(); ++i)
            {
                m_vv4WaterMap[i].x += (m_fEvaporation * 0.1);// 0.000025);
                //m_vv4WaterMap[i].x += (m_fEvaporation * rngMod);// 0.000025);
                
                //m_vv4
                if(pHeight[i] < m_fWaterLevel)
                {
                    //printf("Setting Water Level at %d to %f\n", i, m_fWaterLevel - pHeight[i]);
                    m_vv4WaterMap[i].x = m_fWaterLevel - pHeight[i];
                }
            }
            printf("Finished Adding Water\n");
            bNoWater = false;
        }

        // Calculation Step 

        // uint32_t nThreads = 0;
        // #pragma omp parallel
        // {
        //     nThreads = omp_get_num_threads();
        // }

        // omp_set_num_threads(nThreads * 2);

        #ifdef NDEBUG
        #pragma omp parallel for
        #endif
        for(uint32_t y = 0; y < uHeight; ++y)
        {
            auto my = y * uWidth;

            for(uint32_t x = 0; x < uWidth; ++x)
            {
                // What is the most water we can move?
                auto maximalWaterToMove = m_vv4WaterMap[my+x].x;
                auto terrainHeight = pHeight[my+x];
                auto waterLevel = terrainHeight + maximalWaterToMove;

                // Create stack array of valid neighbours
                FHydFloatCombiner stLowerNeighbours{};

                // Heights
                float sumHeightDifferent = 0;
                auto nLowerNeighbours = 0;

                for(uint32_t n = 0; n < 8; ++n)
                {
                    auto ln = n;
                    if(ln > 3) { ++ln; }

                    int32_t yMod = ln / 3 - 1;
                    int32_t xMod = ln % 3 - 1;
                    
                    //printf("%d means y:%d x:%d\n", n, yMod, xMod);
                    auto yReal = y + yMod;
                    auto xReal = x + xMod;

                    // Bounds
                    if (yReal < 0 || yReal >= uHeight || xReal < 0 || xReal >= uWidth)
                    {
                        continue;
                    }

                    
                    auto checkedWaterLevel = pHeight[yReal * uWidth + xReal] + m_vv4WaterMap[yReal * uWidth + xReal].x;
                    if (checkedWaterLevel >= waterLevel) { continue; }

                    // We have water to move
                    sumHeightDifferent += (waterLevel - checkedWaterLevel);
                    ++nLowerNeighbours;
                    stLowerNeighbours.values[n] = waterLevel - checkedWaterLevel;
                }

                // Is there anything to do?
                //if(nLowerNeighbours == 0) { continue; }

                // Calc DeltaW
                // Or don't
                auto deltaW = maximalWaterToMove;//std::min(maximalWaterToMove, sumHeightDifferent / nLowerNeighbours);
                //auto sedimentSaturation = m_vv4WaterMap[my+x].y / m_vv4WaterMap[my+x].x;
                auto sedimentHere = m_vv4WaterMap[my+x].y;

                for(uint32_t n = 0; n < 8; ++n)
                {
                    // Is Valid Neighbour?
                    if(stLowerNeighbours.values[n] == 0){ continue; }

                    // Calculate the proportional share of the water
                    auto ln = n;
                    if(ln > 3) { ++ln; }

                    int32_t yMod = ln / 3 - 1;
                    int32_t xMod = ln % 3 - 1;
                    
                    auto yReal = y + yMod;
                    auto xReal = x + xMod;

                    // Changed nNeighbours to heightDelta

                    auto ratio = (stLowerNeighbours.values[n] / sumHeightDifferent);
                    auto waterToMoveHere = deltaW * ratio;
                    auto sedToMoveHere = sedimentHere * ratio * 0.75f;

                    // Sediment To Move is function of the sediment in the water


                    // Change Water Level at this pixel
                    m_vv4WaterMap[my+x].z += waterToMoveHere * m_fWorldVerticalScale;
                    m_vv4WaterMap[my+x].w += sedToMoveHere;

                    m_vstWaterCombiner[yReal * uWidth + xReal].values[n] = waterToMoveHere * m_fWorldVerticalScale;
                    m_vstSedimentCombiner[yReal * uWidth + xReal].values[n] = sedToMoveHere;
                }
            }
        }
    

        // omp_set_num_threads(nThreads);

        // Combiner Step

        #ifdef NDEBUG
        #pragma omp parallel for
        #endif
        for(uint32_t x = 0; x < m_vv4WaterMap.size(); ++x)
        {
            auto stack = m_vstWaterCombiner[x].values;
            auto sedStack = m_vstSedimentCombiner[x].values;

            // Handle delayed subtraction of water level and sediment
            // .z is Cs
            // StV is stuff leaving AKA Sediment Lost
            //auto sedimentLost = m_vv4WaterMap[x].z * (m_vv4WaterMap[x].y / m_vv4WaterMap[x].x);
            m_vv4WaterMap[x].x -= m_vv4WaterMap[x].z / m_fWorldVerticalScale;
            m_vv4WaterMap[x].y -= m_vv4WaterMap[x].w;

            if (m_vv4WaterMap[x].y < -0.0001) {printf("WARN: Sed < 0: %f\n", m_vv4WaterMap[x].y);}

            for(uint32_t sp = 0; sp < 8; ++sp)
            {
                m_vv4WaterMap[x].x += stack[sp] / m_fWorldVerticalScale;
                
                // Also add the water we gain
                m_vv4WaterMap[x].z -= stack[sp];

                // Sediment
                // Total Sediment Moved is water * fillLevel
                auto sedimentAmount = sedStack[sp];
                m_vv4WaterMap[x].y += sedimentAmount;

                stack[sp] = 0;
                sedStack[sp] = 0;
            }

            // {printf("TEST: WaterMovement %f\n",fabs(m_vv4WaterMap[x].z));}
            // if (fabs(m_vv4WaterMap[x].z) > 0) {printf("WARN: WaterMovement %f\n",m_vv4WaterMap[x].z);}
            // // Floating Sediment 
            // If waterMoving is == 0
            if(abs(m_vv4WaterMap[x].z) < FLT_EPSILON)
            {
                // Deposit
                pHeight[x] += m_fDeposition * m_vv4WaterMap[x].y;
                m_vv4WaterMap[x].y -= m_fDeposition * m_vv4WaterMap[x].y;
            }



            // We're done with Combining

            // Water Loss due to evaporation is given by
            // -K*W
            // Can be written as w1 = w0 * (1-K)
            auto evaporationLevel = m_fEvaporation * m_vv4WaterMap[x].x;
            m_vv4WaterMap[x].x -= evaporationLevel;

            if(m_vv4WaterMap[x].x < 0.00000625f) { m_vv4WaterMap[x].x = 0; }
            m_vv4WaterMap[x].x = std::max(m_vv4WaterMap[x].x, 0.f);
            m_vv4WaterMap[x].y = std::max(m_vv4WaterMap[x].y, 0.f);
            
            // // More Sediment
            // auto sedCapacity = m_fSedimentCapacity * m_vv4WaterMap[x].x;
            // //if (x == 0) {printf("%f\n", sedCapacity);}
            // if(m_vv4WaterMap[x].y >= sedCapacity)
            // {
            //     // Deposit everything that is over cap
            //     auto deltaS = m_fDeposition * (m_vv4WaterMap[x].y - sedCapacity);
            //     pHeight[x] += deltaS;
            //     m_vv4WaterMap[x].y -= deltaS;
            // }
            // else if(m_vv4WaterMap[x].z >= FLT_EPSILON)
            // {
            //     // Pick up the sediment
            //     float rngMod = 1;// - 0.1 * (double(uint64_t(6364136223846793005 * GetWorldXY(x, uWidth) + 1442695040888963407)) / double(UINT64_MAX));

            //     //printf("%f\n", rngMod);

            //     auto deltaS = m_fSoilSoftness * (sedCapacity - m_vv4WaterMap[x].y) * rngMod;
            //     m_vv4WaterMap[x].y += deltaS;
            //     pHeight[x] -= deltaS;
            // }

            if(fabs(m_vv4WaterMap[x].z) >= FLT_EPSILON)
            {
                // More Sediment
                auto sedCapacity = m_fSedimentCapacity * m_vv4WaterMap[x].x;
                //{printf("%f\n", sedCapacity);}
                if(m_vv4WaterMap[x].y >= sedCapacity)
                {
                    // Deposit everything that is over cap
                    auto deltaS = m_fDeposition * (m_vv4WaterMap[x].y - sedCapacity);
                    pHeight[x] += deltaS;
                    m_vv4WaterMap[x].y -= deltaS;
                }
                else
                {
                    // Pick up the sediment
                    float rngMod = 1;// - 0.1 * (double(uint64_t(6364136223846793005 * GetWorldXY(x, uWidth) + 1442695040888963407)) / double(UINT64_MAX));

                    //printf("%f\n", rngMod);

                    auto deltaS = m_fSoilSoftness * (sedCapacity - m_vv4WaterMap[x].y) * rngMod;
                    m_vv4WaterMap[x].y += deltaS;
                    pHeight[x] -= deltaS;
                }
            }

            // Reset the delayed amount
            m_vv4WaterMap[x].z = 0;
            m_vv4WaterMap[x].w = 0;


            //printf("%f\n", m_vv4WaterMap[x].z);

            // + m_vv4WaterMap[x].x;//) / 2.0; 
            //pOut[x] = m_vv4WaterMap[x].x;
            //pOut[x] = m_vv4WaterMap[x].y;
            //pOut[x] = stack->values[1];// pHeight[x] + m_vv4WaterMap[x].x; 
        }

    //     // Sediment Prep Step
    //     #ifdef NDEBUG
    //     #pragma omp parallel for
    //     #endif
    //     for(uint32_t y = 0; y < uHeight; ++y)
    //     {
    //         auto my = y * uWidth;

    //         for(uint32_t x = 0; x < uWidth; ++x)
    //         {
    //             // Prep Smooth
    //             auto mx = x;
    //             //printf("At %d %d\n", ry, rx);
                
    //             uint32_t nNeighbours = 0;
    //             float fAvgSilt = 0;

    //             for(uint32_t ln = 0; ln < 9; ++ln)
    //             {
    //                 // Calculate the proportional share of the water

    //                 int32_t yMod = ln / 3 - 1;
    //                 int32_t xMod = ln % 3 - 1;
                    
    //                 int32_t yReal = y + yMod;
    //                 int32_t xReal = x + xMod;

    //                 // Bounds
    //                 if (yReal < 0 || yReal >= uHeight || xReal < 0 || xReal >= uWidth)
    //                 {
    //                     continue;
    //                 }
                    
    //                 fAvgSilt += m_vv4WaterMap[yReal * uWidth + xReal].y;
    //                 ++nNeighbours;
    //             }

    //             // Worst case, this is 1
    //             if ( x==0 && y==0) {printf("%f\n", fAvgSilt / nNeighbours);}
    //             m_vv4WaterMap[my+x].z = fAvgSilt / nNeighbours;
    //         }
    //     }

    //     // Sediment Equaliser Step
    //     #ifdef NDEBUG
    //     #pragma omp parallel for
    //     #endif
    //     for(uint32_t x = 0; x < m_vv4WaterMap.size(); ++x)
    //     {
    //         m_vv4WaterMap[x].y = m_vv4WaterMap[x].z * 0.5;
    //         //m_vv4WaterMap[x].y *= 0.5f;
    //         m_vv4WaterMap[x].z = 0;
    //     }
    
    }

    // Sediment Equaliser Step
    #ifdef NDEBUG
    #pragma omp parallel for
    #endif
    for(uint32_t x = 0; x < m_vv4WaterMap.size(); ++x)
    {
        // Drop Sediment in place
        pOut[x] = pHeight[x] + m_vv4WaterMap[x].y;
    }
}

void CHydraulicErosion::Erode(FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uHeight, uint32_t uWidth, float fScale)
{
    // New Function, let's detour
    TestFunc(pHeight, pOut, uHeight, uWidth, fScale);
    fprintf(stderr, "Iteration Finished\n");
    
    return;


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

                    auto ero = Gradient.x * fScale * 0.25f;

                    pOut[my + x] = CentrePixel + ero;
                    //pOut[my + x] = ero;
                    // pOut[my + x] = Gradient.x * 1024.f;

                    // // Erosion Based on normals
                    // auto rT = pHeight[(y+1) * uWidth + x] * 255;
                    // auto rB = pHeight[(y-1) * uWidth + x] * 255;
                    // auto rR = pHeight[(y) * uWidth + (x+1)] * 255;
                    // auto rL = pHeight[(y) * uWidth + (x-1)] * 255;

                    // auto pointNorm = glm::normalize(glm::vec3(2 * (rL-rR), 4, 2 * (rB-rT)));
                    // auto rawAngle = glm::dot(pointNorm, glm::vec3(0,1,0));
                    // auto asDeg = acos(rawAngle) * (2.f/3.1415926535);
                    // auto asRemap = 1 - (fabs(0.5f - asDeg) * 2.f);
                    // auto mulPt = (1 - 0.5 * glm::dot(pointNorm, glm::vec3(0,1,0)));
                    
                    // //pOut[my + x] = CentrePixel - asRemap * 0.001f;

                    // pOut[my + x] = CentrePixel + Gradient.x * fScale;


                    //pOut[my + x] = asRemap;

                    // if(asRemap > 0.5f)
                    // {
                    //     pOut[my + x] = CentrePixel + asRemap * Gradient.x * 2.f * 1024.f;// + Gradient.x * 2.f * 1024.f;
                    // }
                    // else
                    // {
                    //     pOut[my + x] = CentrePixel;// + mulPt * Gradient.x * 2.f * 1024.f;// + Gradient.x * 2.f * 1024.f;//CentrePixel + Gradient.x * 1024.f * mulPt;
                    // }

                    // if(Gradient.x < 0.f)
                    // {
                    //     pOut[my + x] = asDeg;// + asRemap * Gradient.x * 2.f * 1024.f;// + Gradient.x * 2.f * 1024.f;
                    // }
                    // else
                    // {
                    //     pOut[my + x] = asDeg;//CentrePixel;// + mulPt * Gradient.x * 2.f * 1024.f;// + Gradient.x * 2.f * 1024.f;//CentrePixel + Gradient.x * 1024.f * mulPt;
                    // }

                    // pOut[my + x] = CentrePixel + ero;
                }
            }
        }
    }


    // Smooth
#ifdef NDEBUG
#pragma omp parallel for
#endif
    for(uint32_t y = 0; y < uHeight; ++y)
    {
        auto my = y * uWidth;

        if(y < m_iOverscan * 2 || y >= uHeight - m_iOverscan * 2)
        {
            //std::memcpy(&pOut[my], &pOut[my], sizeof(uint32_t) * uWidth);
        }
        else
        {
            for(uint32_t x = 0; x < uWidth; ++x)
            {
                if(x < m_iOverscan * 2 || x >= uWidth - m_iOverscan * 2)
                {
                    //pOut[my + x] = pOut[my + x];
                }
                else
                {
                    // Guassian Smooth 9-tab SQ (max8)
                    // Total Weight - 8
                    auto CentrePixel = pOut[my + x] * 8;

                    // Total Weight - 32
                    auto rT = pOut[(y+1) * uWidth + x] * 8;
                    auto rB = pOut[(y-1) * uWidth + x] * 8;
                    auto rR = pOut[(y) * uWidth + (x+1)] * 8;
                    auto rL = pOut[(y) * uWidth + (x-1)] * 8;

                    // Total Weight - 16
                    auto rTT = pOut[(y+2) * uWidth + x] * 2;
                    auto rBB = pOut[(y-2) * uWidth + x] * 2;
                    auto rRR = pOut[(y) * uWidth + (x+2)] * 2;
                    auto rLL = pOut[(y) * uWidth + (x-2)] * 2;
                    auto rTR = pOut[(y+1) * uWidth + (x+1)] * 2;
                    auto rTL = pOut[(y+1) * uWidth + (x-1)] * 2;
                    auto rBR = pOut[(y-1) * uWidth + (x+1)] * 2;
                    auto rBL = pOut[(y-1) * uWidth + (x-1)] * 2;

                    // Total Weight - 8
                    auto rTTR = pOut[(y+2) * uWidth + (x+1)];
                    auto rTTL = pOut[(y+2) * uWidth + (x-1)];
                    auto rBBR = pOut[(y-2) * uWidth + (x+1)];
                    auto rBBL = pOut[(y-2) * uWidth + (x-1)];
                    auto rTRR = pOut[(y+1) * uWidth + (x+2)];
                    auto rTLL = pOut[(y+1) * uWidth + (x-2)];
                    auto rBRR = pOut[(y-1) * uWidth + (x+2)];
                    auto rBLL = pOut[(y-1) * uWidth + (x-2)];

                    auto sum = CentrePixel + rT + rB + rR + rL + rTR + rTL + rBR + rBL + rTT + rBB + rLL + rRR + rTTR + rTTL + rBBR + rBBL + rTRR + rTLL + rBRR + rBLL;

                    pOut[my+x] = sum / 64.f;
                }
            }
        }
    }

}
