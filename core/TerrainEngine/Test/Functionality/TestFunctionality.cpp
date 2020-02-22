#include <iostream>
#include "core/TerrainEngine/Private/Header/TerrainEngine_Impl.h"
#include <fstream>


int main(int argc, char** argv)
{
    auto terrainEngine = CTerrainEngine_Impl(250, 1000, 1000);

    terrainEngine.EnableImmediateMode();

    terrainEngine.CreateLayer();
    terrainEngine.CreateLayer();

    terrainEngine.Perlin(0, 1);
    //terrainEngine.MulLayerScalar(0, 0, 1);
    // terrainEngine.Perlin(1, 0.25);
    // terrainEngine.Perlin(2, 10);

    // terrainEngine.AddLayers(0, 0, 1);
    // terrainEngine.AddLayers(0, 0, 2);
    // terrainEngine.MulLayerScalar(0, 0, 0.33);

    // Show view holding locks
    {
        auto view = terrainEngine.GetView();

        std::cout << "Holding " << view.use_count() << " views to the array" << std::endl;
        //std::cout << view->size() << std::endl;

        terrainEngine.Erode(0, 11);

        std::cout << "Holding " << view.use_count() << " views to the array" << std::endl;
    }

    

    //std::cout << view->at(2) << std::endl;
    //std::cout << view->at(9 * 1001 + 234) << std::endl;



    // I'm bored, let's write to disk :)
    {
        auto view = terrainEngine.GetView();

        std::ofstream of("out.hgt", std::ios::binary);
        auto lPtr = *view.get();
        of.write(reinterpret_cast<char*>(&lPtr[0]), view->size() * sizeof(lPtr[0]));
    }

    return 0;
}