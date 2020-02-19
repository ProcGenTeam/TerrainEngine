#include <iostream>
#include "core/Public/engine/Engine.h"
#include "core/Private/engine/Engine_Impl.h"

int main(int argc, char** argv)
{
    auto terrainEngine = CTerrainEngine_Impl(250, 512, 512);
    auto view = terrainEngine.GetView();

    std::cout << "Holding " << view.use_count() << " views to the array" << std::endl;
    std::cout << view->size() << std::endl;


    std::cout << "Hello :)" << std::endl;
    return 0;
}