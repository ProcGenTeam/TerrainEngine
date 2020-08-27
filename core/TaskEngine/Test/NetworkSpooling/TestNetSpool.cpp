#include "core/TaskEngine/Private/Header/TaskEngine_Impl.h"
#include "core/TerrainEngine/Public/Header/TerrainEngine.h"
#include <fstream>
#include <iostream>

int main(int argc, char **argv)
{
    auto teng = TerrainEngine::CTerrainEngine(250, 512, 512);
    teng.Perlin(0, 10);
    auto hist = teng.GetHistory();

    auto ttest = CTaskEngine_Impl(20001, 20001, 4096);

    auto ret = ttest.Listen(6787);
    std::cout << "Listen Result: " << GetName_en_gb(ret) << std::endl;

    ttest.RegisterNode("127.0.0.1", 6787);
    std::cout << "Register Result: " << GetName_en_gb(ret) << std::endl;

    ttest.Render(hist);

    return 0;
}