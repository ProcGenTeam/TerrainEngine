#include <iostream>
#include "core/TerrainEngine/Public/Header/TerrainEngine.h"
#include "core/TaskEngine/Public/Header/TaskEngine.h"
//#include "core/TerrainEngine/Private/Header/Engine_Impl.h"
#include <fstream>


int main(int argc, char** argv)
{
    auto terrainEngine = TerrainEngine::CTerrainEngine(250, 512, 512);
    auto tasks = TerrainEngine::CTaskEngine(20001, 20001);

    tasks.RegisterNode("192.168.1.1", 6556);

    // 

    auto view = terrainEngine.GetView(0);

    std::cout << "Holding " << view.use_count() << " views to the array" << std::endl;
    std::cout << view->size() << std::endl;

    terrainEngine.Erode(0, 100);

    std::cout << view->at(2) << std::endl;

    std::cout << "Hello :)" << std::endl;

    // I'm bored, let's write to disk :)
    {
        std::ofstream of("out.hgt", std::ios::binary);
        auto lPtr = *view.get();
        of.write(reinterpret_cast<char*>(&lPtr[0]), view->size() * sizeof(lPtr[0]));
    }


    return 0;
}