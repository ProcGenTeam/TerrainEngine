cmake_minimum_required(VERSION 3.7)
project(TerrainEngineProject)

set(CMAKE_CXX_STANDARD 17)

find_package( Threads )
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

set(USE_CUDA false CACHE BOOL "Enable CUDA")
if (USE_CUDA)
    add_definitions(-DTE_USE_CUDA)
endif(USE_CUDA)

set(FRONT_END
    main.cpp
)

include_directories(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/core
)

add_executable(TerrainEnginert ${FRONT_END})


target_link_libraries(TerrainEnginert
    TerrainEngine
    ${CMAKE_THREAD_LIBS_INIT}
)

## Add Lib
add_subdirectory(core/TerrainEngine)