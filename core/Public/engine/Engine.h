#include "core/Public/common/defines.h"
#include <cstdint>

TE_NS_OPEN

class CTerrainEngine
{
    private:
        void *m_implementation;
    public:
    // Methods
        CTerrainEngine(uint32_t uWaterLevel, uint32_t uWidth, uint32_t uHeight, float fScale = 0.001f);
        ~CTerrainEngine();
};

TE_NS_CLOSE