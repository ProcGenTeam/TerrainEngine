#include <iostream>
#include "core/TerrainEngine/Private/Header/TerrainEngine_Impl.h"
#include "core/RenderEngine/Public/Header/RenderEngine.h"
#include <fstream>


int main(int argc, char** argv)
{
    float worldScale = 0.15;

    uint32_t baseRes = 256;

    for(int i = 0; i < 1; ++i)
    {
        uint32_t x = i / 2;
        uint32_t y = i % 2;

        std::cout << x << " " << y << std::endl;

        auto terrainEngine = CTerrainEngine_Impl(
            0.01f,
            baseRes,
            baseRes,
            baseRes * x,
            baseRes * y,
            worldScale,
            32,
            2
        );

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
        terrainEngine.ErodeByNormals(0, 1);

        // terrainEngine.MulLayerScalar(0,0,0);
        terrainEngine.Perlin(1, 25);
        terrainEngine.Perlin(2, 4);
        terrainEngine.MulLayerScalar(1, 1, 0.0025);
        terrainEngine.MulLayers(1, 1, 2);
        terrainEngine.AddLayers(0, 0, 1);

        //terrainEngine.MulLayerScalar(0,0,0);
        terrainEngine.Perlin(1, 120);
        terrainEngine.MulLayerScalar(1, 1, 0.0005);
        terrainEngine.AddLayers(0,0,1);



        // terrainEngine.MulLayerScalar(0, 0, 0.33);

        // Show view holding locks
        {
            auto view = terrainEngine.GetView(0);

            std::cout << "Holding " << view.use_count() << " views to the array" << std::endl;
            //std::cout << view->size() << std::endl;

            terrainEngine.Erode(0, 5);

            // for(uint32_t x = 0; x < 5; ++x)
            // {
            //     terrainEngine.Erode(0, 1);
            //     terrainEngine.ErodeByNormals(0, 8);
            //     std::cout << "Limit: " << x << std::endl;
            // }

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

            std::ofstream of(std::to_string(x) + "," + std::to_string(y) + ".hgt", std::ios::binary);
            auto lPtr = *view.get();
            of.write(reinterpret_cast<char*>(&lPtr[0]), view->size() * sizeof(lPtr[0]));
        }

        {
            FCameraSettings st{};
            st.uWidth = st.uHeight = 1024;
            st.fSensorSizeX = st.fSensorSizeY = 0.1;
            st.fCameraDistance = 0.1f;
            st.fStepDistance = 0.1f;
            st.uWorldScaleUnits = 1024;
            st.fWorldHeightRenderScale = 100.f / worldScale;
            st.fFarClip = 10000.f;
            st.fNearClip = 0.01f;
            auto view = terrainEngine.GetView(0);

            auto rend = CRenderEngine(st);
            rend.MarchedRender("skyHigh_" + std::to_string(x) + "_" + std::to_string(y) + ".rgb", view, baseRes + 64);
        }

    }
    return 0;
}
