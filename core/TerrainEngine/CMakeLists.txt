cmake_minimum_required(VERSION 3.7)
project(TerrainEngine VERSION 0.0.1 DESCRIPTION "Terrain Generation Component")
set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

# Switch development flags
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall --ggdb")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

if (USE_GPU)
    find_package(Vulkan)
    if(VULKAN_FOUND)
        add_definitions(-DTE_USE_GPU)
    else(VULKAN_FOUND)
        message(FATAL_ERROR "GPGPU was requested in the build, but Vulkan is not available. Exiting.")
    endif(VULKAN_FOUND)
endif(USE_GPU)


# Files
set(PrivateHeaders
    Private/Header/GPUHydraulicErosion.h
    Private/Header/TerrainEngine_Impl.h
    Private/Header/PerlinNoise.h
    Private/Header/HydraulicErosion.h
)

set(PrivateSources
    Private/Source/GPUHydraulicErosion.cpp
    Private/Source/TerrainEngine_Impl.cpp
    Private/Source/TerrainEngine_Impl_Internal.cpp
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
    ${CMAKE_SOURCE_DIR}/extern/glm
    ${Vulkan_INCLUDE_DIRS}
)

target_include_directories(TerrainEngine PRIVATE
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(
    TerrainEngine
    glm
    ${CMAKE_THREAD_LIBS_INIT}
    ${Vulkan_LIBRARIES}
)

add_subdirectory(Test)
#add_subdirectory(${CMAKE_SOURCE_DIR}/extern/glm ./glm)
