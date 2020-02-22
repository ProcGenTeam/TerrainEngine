cmake_minimum_required(VERSION 3.7)
project(TerrainEngine VERSION 0.0.1 DESCRIPTION "Terrain Generation Component")
set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

# Switch development flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

# Files
set(PrivateHeaders
    Private/Header/CuEngine_Impl.h
    Private/Header/TerrainEngine_Impl.h
    Private/Header/PerlinNoise.h
    Private/Header/HydraulicErosion.h
    Private/Header/Common.h
)

set(PrivateSources
    Private/Source/CuEngine_Impl.cu
    Private/Source/TerrainEngine_Impl.cpp
    Private/Source/TerrainEngine.cpp
    Private/Source/PerlinNoise.cpp
    Private/Source/HydraulicErosion.cpp
)

set(PublicHeaders
    Public/Header/Defines.h
    Public/Header/TerrainEngine.h
    Public/Header/Operations.h
)

# Add Shared Object
add_definitions(-D__TERRAIN_ENGINE_EXPORTS__)
add_library(TerrainEngine SHARED ${PublicHeaders} ${PrivateHeaders} ${PrivateSources})
set_target_properties(TerrainEngine PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(TerrainEngine PROPERTIES SOVERSION 1)
set_target_properties(TerrainEngine PROPERTIES LINKER_LANGUAGE CXX)

link_directories(
    TerrainEngine
)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

target_include_directories(TerrainEngine PRIVATE
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(
    TerrainEngine
    glm
    ${CMAKE_THREAD_LIBS_INIT}
)

add_subdirectory(Test)
add_subdirectory(${CMAKE_SOURCE_DIR}/extern/glm ./glm)