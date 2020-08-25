cd "${0%/*}"
#cd ./core/TerrainEngine/Private
#find . -name "*.comp" -exec glslangValidator -V {} -o ./build/{}.spv \;

glslangValidator -V ./core/TerrainEngine/Private/Shader/Rain.comp -o build/Rain.spv
glslangValidator -V ./core/TerrainEngine/Private/Shader/Erode.comp -o build/Erode.spv
glslangValidator -V ./core/TerrainEngine/Private/Shader/DumpSediment.comp -o build/Dump.spv
glslangValidator -V ./core/TerrainEngine/Private/Shader/Combine.comp -o build/Combine.spv
