#include <iostream>
#include "core/TerrainEngine/Private/Header/TerrainEngine_Impl.h"
#include <fstream>


int main(int argc, char** argv)
{
    auto terrainEngine = CTerrainEngine_Impl(250, 4096, 4096);

    terrainEngine.EnableImmediateMode();

    auto view = terrainEngine.GetView();

    std::cout << "Holding " << view.use_count() << " views to the array" << std::endl;
    std::cout << view->size() << std::endl;

    terrainEngine.Perlin(0);

    //terrainEngine.Erode(100);

    std::cout << view->at(2) << std::endl;


    // I'm bored, let's write to disk :)
    {
        std::ofstream of("out.hgt", std::ios::binary);
        auto lPtr = *view.get();
        of.write(reinterpret_cast<char*>(&lPtr[0]), view->size() * sizeof(lPtr[0]));
    }

    return 0;
}