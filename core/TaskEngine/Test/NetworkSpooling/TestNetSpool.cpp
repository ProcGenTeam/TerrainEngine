#include <iostream>
#include "core/TaskEngine/Private/Header/TaskEngine_Impl.h"
#include "core/TerrainEngine/Public/Header/TerrainEngine.h"
#include <fstream>

int main(int argc, char** argv)
{
    auto teng = TerrainEngine::CTerrainEngine(250, 512, 512);
    teng.Perlin(0, 10);
    auto hist = teng.GetHistory();


    auto ttest = CTaskEngine_Impl(20001, 20001, 4096);

    ttest.Listen(6787);

    ttest.RegisterNode("127.0.0.1", 6787);

    ttest.Render(hist);
     
    return 0;
}