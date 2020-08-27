#pragma once
#include "Common/Public/Header/Types.h"
#include <cstdint>
#include <vector>
#include <random>

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#include "TerrainEngine/Public/Header/Defines.h"

struct FGradSumParams
{
    FLOAT_TYPE *pHeight;

    FLOAT_TYPE *pOut;

    uint32_t uX;
    uint32_t uY;

    uint32_t uWidth;
    uint32_t uHeight;

    float fOneOverWidth;
    float fOneOverHeight;
};

struct FExitGradParams
{
    FLOAT_TYPE *pHeight;

    uint32_t uX;
    uint32_t uY;

    uint32_t uWidth;
    uint32_t uFilterWidth;

    float fOneOverWidth;
    float fOneOverHeight;
};

struct FHydFloatCombiner
{
    float values[8];
};

struct FHydUIntCombiner
{
    uint32_t values[8];
};
static_assert(sizeof(FHydFloatCombiner) == 8*4);

class CHydraulicErosion
{
    protected:
        int32_t m_iOverscan;
        FLOAT_TYPE m_fWaterLevel;

        FLOAT_TYPE m_fEvaporation;
        FLOAT_TYPE m_fSedimentCapacity;
        FLOAT_TYPE m_fDeposition;
        FLOAT_TYPE m_fSoilSoftness;

        FLOAT_TYPE m_fWorldVerticalScale;

        int32_t m_iOffsetX;
        int32_t m_iOffsetY;

        std::mt19937_64 m_mtRandGen;
        std::normal_distribution<float> m_distNormal;
        std::vector<glm::vec3> m_vv3GradientMap;
        std::vector<glm::vec4> m_vv4WaterMap;

        std::vector<FHydFloatCombiner> m_vstWaterCombiner;
        std::vector<FHydFloatCombiner> m_vstSedimentCombiner;

        virtual glm::vec3 GradientSum(FGradSumParams &stParams);
            //FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uX, uint32_t uY, uint32_t uWidth, uint16_t uFilterWidth);

        virtual glm::vec3 ExitGradients(FGradSumParams &stParams);
            //FLOAT_TYPE *pHeight, uint32_t uWidth, uint32_t uX, uint32_t uY);

        virtual void GenerateGradientMap(FLOAT_TYPE *pHeight, uint32_t uWidth, uint32_t uHeight);

    public:
        CHydraulicErosion(int32_t iOverscan, uint32_t uSeed, int32_t iOffsetX, int32_t iOffsetY, FLOAT_TYPE fWaterLevel = 0.1f);
        virtual ~CHydraulicErosion();

        virtual void TestFunc(FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uHeight, uint32_t uWidth, uint32_t uStop, FLOAT_TYPE *pTerrain, float fScale = 0.1f);

        virtual void Erode(FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uHeight, uint32_t uWidth, uint32_t uSteps, FLOAT_TYPE *pTerrain, float fScale = 0.1f);
        virtual void ErodeByNormals(FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uHeight, uint32_t uWidth, float fScale = 0.1f);
        //virtual void Erode(FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uHeight, uint32_t uWidth, float fScale = 0.1f);
};