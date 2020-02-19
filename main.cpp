#include <iostream>
#include "core/TerrainEngine/Public/Header/Engine.h"
//#include "core/TerrainEngine/Private/Header/Engine_Impl.h"

int main(int argc, char** argv)
{
    auto terrainEngine = TerrainEngine::CTerrainEngine(250, 512, 512);
    auto view = terrainEngine.GetView();

    std::cout << "Holding " << view.use_count() << " views to the array" << std::endl;
    std::cout << view->size() << std::endl;

    terrainEngine.Erode(100);


    std::cout << "Hello :)" << std::endl;
    return 0;
}