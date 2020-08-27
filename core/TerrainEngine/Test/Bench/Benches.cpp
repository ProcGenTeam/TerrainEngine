#include "core/RenderEngine/Public/Header/RenderEngine.h"
#include "core/TerrainEngine/Private/Header/TerrainEngine_Impl.h"
#include <chrono>
#include <fstream>
#include <iostream>

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " <resolution> <iterations>" << std::endl;
        return 255;
    }

    float worldScale = 0.15;
    uint32_t baseRes = std::stoull(argv[1]);
    uint32_t iterations = std::stoull(argv[2]);

    auto terrainEngine = CTerrainEngine_Impl(0.125f, baseRes, baseRes, 0, 0, worldScale, 32, 2);

    // Enable Immediate. We want to just build everything
    terrainEngine.EnableImmediateMode();

    terrainEngine.CreateLayer();
    terrainEngine.Perlin(0, 1);
    terrainEngine.Perlin(1, 2.5);
    terrainEngine.MulLayers(0, 0, 1);

    terrainEngine.CreateLayer();
    terrainEngine.Perlin(1, 0.78);
    terrainEngine.Perlin(2, 4);
    terrainEngine.MulLayerScalar(2, 2, 0.15);
    terrainEngine.SubLayers(1, 1, 2);

    terrainEngine.MulLayerScalar(1, 1, 0.2);
    terrainEngine.AddLayers(0, 0, 1);

    // Hitting Early
    terrainEngine.ErodeByNormals(0, 10);

    // //terrainEngine.MulLayerScalar(0,0,0);
    terrainEngine.Perlin(1, 25);
    terrainEngine.Perlin(2, 4);
    terrainEngine.MulLayerScalar(1, 1, 0.025);
    terrainEngine.MulLayers(1, 1, 2);
    terrainEngine.AddLayers(0, 0, 1);

    auto startTime = std::chrono::high_resolution_clock::now();

    // Show view holding locks
    for (uint32_t i = 0; i < 1; ++i)
    {
        terrainEngine.Erode(0, iterations);
    }

    auto endTime = std::chrono::high_resolution_clock::now();

    auto timeTakenUS = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();

    std::cout << std::endl;
    std::cout << "RESULTS" << std::endl;
    std::cout << "-------" << std::endl;

    std::cout << "Time Total: " << timeTakenUS / 1000000.f << " s" << std::endl;
    std::cout << "S per Iteration: " << (timeTakenUS / iterations) / 1000000.f << " s" << std::endl;
    std::cout << "uS per Iteration per Pixel: " << (timeTakenUS / iterations) / (float(baseRes * baseRes)) << " uS"
              << std::endl;

    std::cout << std::endl;

    std::cout << "Peak Mem Use: " << terrainEngine.GetPeakMemoryUse() / (1024.0 * 1024.0) << " MiB" << std::endl;
    std::cout << "Peak Layer Mem Use: "
              << terrainEngine.GetPeakMemoryUse(EMemoryUseTypes::LayerMemory) / (1024.0 * 1024.0) << " MiB"
              << std::endl;
    std::cout << "Peak Transient Method Mem Use: "
              << terrainEngine.GetPeakMemoryUse(EMemoryUseTypes::MethodMemory) / (1024.0 * 1024.0) << " MiB"
              << std::endl;

    return 0;
}
