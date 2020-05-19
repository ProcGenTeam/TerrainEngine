#pragma once
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

class CHydraulicErosion
{
    private:
        int32_t m_iOverscan;
        std::mt19937_64 m_mtRandGen;
        std::normal_distribution<float> m_distNormal;
        std::vector<glm::vec3> m_vv3GradientMap;

        virtual glm::vec3 GradientSum(FGradSumParams &stParams);
            //FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uX, uint32_t uY, uint32_t uWidth, uint16_t uFilterWidth);

        virtual glm::vec3 ExitGradients(FGradSumParams &stParams);
            //FLOAT_TYPE *pHeight, uint32_t uWidth, uint32_t uX, uint32_t uY);

        virtual void GenerateGradientMap(FLOAT_TYPE *pHeight, uint32_t uWidth, uint32_t uHeight);

    public:
        CHydraulicErosion(int32_t iOverscan, uint32_t uSeed);
        ~CHydraulicErosion();

        virtual void Erode(FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uHeight, uint32_t uWidth, float fScale = 0.1f);
        //virtual void Erode(FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uHeight, uint32_t uWidth, float fScale = 0.1f);
};