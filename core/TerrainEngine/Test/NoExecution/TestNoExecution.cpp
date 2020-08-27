#include "core/TerrainEngine/Private/Header/TerrainEngine_Impl.h"
#include <iostream>

int main(int argc, char **argv)
{
    for (uint32_t i = 1; i < 20; ++i)
    {
        std::cout << "Testing Memory Usage for " << i << "k images" << std::endl;

        auto terrainEngine = CTerrainEngine_Impl(250, i * 1024, i * 1024, 0, 0, 1.0, 16, 2);

        terrainEngine.CreateLayer();
        terrainEngine.Perlin(0, 1);
        terrainEngine.Perlin(1, 2.5);
        terrainEngine.MulLayers(0, 0, 1);

        terrainEngine.CreateLayer();
        terrainEngine.Perlin(1, 0.78);
        terrainEngine.Perlin(2, 10);
        terrainEngine.MulLayerScalar(2, 2, 0.15);
        terrainEngine.SubLayers(1, 1, 2);

        terrainEngine.MulLayerScalar(1, 1, 0.2);
        terrainEngine.AddLayers(0, 0, 1);

        // terrainEngine.MulLayerScalar(0,0,0);
        terrainEngine.Perlin(1, 25);
        terrainEngine.MulLayerScalar(1, 1, 0.025);
        terrainEngine.AddLayers(0, 0, 1);

        terrainEngine.Erode(0, 2);

        std::cout << "\tPeak Mem Use: " << terrainEngine.GetPeakMemoryUse() / (1024.0 * 1024.0) << " MiB" << std::endl;
        std::cout << "\tPeak Layer Mem Use: "
                  << terrainEngine.GetPeakMemoryUse(EMemoryUseTypes::LayerMemory) / (1024.0 * 1024.0) << " MiB"
                  << std::endl;
        std::cout << "\tPeak Transient Method Mem Use: "
                  << terrainEngine.GetPeakMemoryUse(EMemoryUseTypes::MethodMemory) / (1024.0 * 1024.0) << " MiB"
                  << std::endl;
    }

    return 0;
}