#pragma once
#ifdef TE_USE_GPU
#include "HydraulicErosion.h"


class CGPUHydraulicErosion : public CHydraulicErosion
{
    private:
    public:
        CGPUHydraulicErosion(int32_t iOverscan, uint32_t uSeed, FLOAT_TYPE fWaterLevel = 0.1f);
        ~CGPUHydraulicErosion();

        virtual void Erode(FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uHeight, uint32_t uWidth, float fScale = 0.1f);
        virtual void ErodeByNormals(FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uHeight, uint32_t uWidth, float fScale = 0.1f);
        //virtual void Erode(FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uHeight, uint32_t uWidth, float fScale = 0.1f);
};


#endif