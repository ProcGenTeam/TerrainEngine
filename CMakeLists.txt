cmake_minimum_required(VERSION 3.7)
project(TerrainEngine)

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

set(COMMON_SOURCE_FILES
        core/Public/common/defines.h
        )

set(PUBLIC_SOURCE_FILES
        core/Public/engine/Engine.h
        core/Public/renderer/Renderer.h
        )

set(PRIVATE_SOURCE_FILES
        core/Private/engine/CuEngine_Impl.h
        core/Private/engine/CuEngine_Impl.cu
        core/Private/engine/Engine_Impl.h
        core/Private/engine/Engine_Impl.cpp
        )

set(FRONT_END
    main.cpp
)

include_directories(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/core
)


#add_library(TerrainEngine ${COMMON_SOURCE_FILES} ${PUBLIC_SOURCE_FILES} ${PRIVATE_SOURCE_FILES})

add_executable(TerrainEnginert main.cpp ${COMMON_SOURCE_FILES} ${PUBLIC_SOURCE_FILES} ${PRIVATE_SOURCE_FILES})


target_link_libraries(TerrainEnginert
    ${CMAKE_THREAD_LIBS_INIT}
)
