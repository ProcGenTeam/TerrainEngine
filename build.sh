mkdir build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DUSE_GPU=True ..
sh ../buildShader.sh
ninja
./core/TerrainEngine/Test/Functionality/FunctionalTests 
python3 ../Render.py skyHigh_0_0.rgb
python3 ../HeightToPng.py 0,0.hgt
