#include <iostream>
#include "core/TerrainEngine/Private/Header/TerrainEngine_Impl.h"
#include <fstream>

std::shared_ptr<std::vector<float>> GenWorld(bool bUseEager)
{
    auto terrainEngine = CTerrainEngine_Impl(
        250,
        256,
        256,
        0,
        0,
        1.0,
        16,
        2
    );

    if(bUseEager) { terrainEngine.EnableImmediateMode(); }

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

    terrainEngine.Perlin(1, 25);
    terrainEngine.MulLayerScalar(1, 1, 0.025);
    terrainEngine.AddLayers(0, 0, 1);


    // This function has threading in release mode
    terrainEngine.Erode(0, 1);

    std::cout << "Peak Mem Use: " << terrainEngine.GetPeakMemoryUse() / (1024.0 * 1024.0) << " MiB" << std::endl;    
    std::cout << "Peak Layer Mem Use: " << terrainEngine.GetPeakMemoryUse(EMemoryUseTypes::LayerMemory) / (1024.0 * 1024.0) << " MiB" << std::endl; 
    std::cout << "Peak Transient Method Mem Use: " << terrainEngine.GetPeakMemoryUse(EMemoryUseTypes::MethodMemory) / (1024.0 * 1024.0) << " MiB" << std::endl; 

    if(!bUseEager) { terrainEngine.Render();}

    return terrainEngine.GetView(0);
}

int main(int argc, char** argv)
{
    auto eager = GenWorld(true);
    auto lazy = GenWorld(false);

    auto misMatch = false;

    for(uint32_t i = 0; i < lazy->size(); ++i)
    {
        if(eager->at(i) != lazy->at(i))
        {
            misMatch = true;
        }
    }

    if(misMatch)
    {
        std::cout << "Test failed" << std::endl;
    }

    return !misMatch;
}