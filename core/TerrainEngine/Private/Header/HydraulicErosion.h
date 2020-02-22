#include <cstdint>
#include <vector>
#include <random>
#include "extern/glm/glm/glm.hpp"
#include "TerrainEngine/Public/Header/Defines.h"

class CHydraulicErosion
{
    private:
        uint32_t m_uOverscan;
        std::mt19937_64 m_mtRandGen;
        std::normal_distribution<float> m_distNormal;


        virtual glm::vec3 GradientSum(FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uX, uint32_t uY, uint32_t uWidth, uint16_t uFilterWidth);

    public:
        CHydraulicErosion(uint32_t uOverscan, uint32_t uSeed);
        ~CHydraulicErosion();

        virtual void Erode(FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uHeight, uint32_t uWidth, float fScale = 0.1f);
};