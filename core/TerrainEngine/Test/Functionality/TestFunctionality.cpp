#include <iostream>
#include "core/TerrainEngine/Private/Header/TerrainEngine_Impl.h"
#include <fstream>


int main(int argc, char** argv)
{
    auto terrainEngine = CTerrainEngine_Impl(250, 1024, 1024, 0, 0, 1.0, 16, 2);

    terrainEngine.EnableImmediateMode();

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

    //terrainEngine.MulLayerScalar(0,0,0);
    terrainEngine.Perlin(1, 25);
    terrainEngine.MulLayerScalar(1, 1, 0.025);
    terrainEngine.AddLayers(0, 0, 1);


    // terrainEngine.MulLayerScalar(0, 0, 0.33);

    // Show view holding locks
    {
        auto view = terrainEngine.GetView(0);

        std::cout << "Holding " << view.use_count() << " views to the array" << std::endl;
        //std::cout << view->size() << std::endl;

        terrainEngine.Erode(0, 2);

        std::cout << "Holding " << view.use_count() << " views to the array" << std::endl;
    }

    std::cout << "Peak Mem Use: " << terrainEngine.GetPeakMemoryUse() / (1024.0 * 1024.0) << " MiB" << std::endl;    
    std::cout << "Peak Layer Mem Use: " << terrainEngine.GetPeakMemoryUse(EMemoryUseTypes::LayerMemory) / (1024.0 * 1024.0) << " MiB" << std::endl; 
    std::cout << "Peak Transient Method Mem Use: " << terrainEngine.GetPeakMemoryUse(EMemoryUseTypes::MethodMemory) / (1024.0 * 1024.0) << " MiB" << std::endl; 

    //std::cout << view->at(2) << std::endl;
    //std::cout << view->at(9 * 1001 + 234) << std::endl;



    // I'm bored, let's write to disk :)
    {
        auto view = terrainEngine.GetView(0);

        std::ofstream of("out.hgt", std::ios::binary);
        auto lPtr = *view.get();
        of.write(reinterpret_cast<char*>(&lPtr[0]), view->size() * sizeof(lPtr[0]));
    }

    return 0;
}