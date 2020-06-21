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

set(USE_GPU false CACHE BOOL "Enable the GPU")
if (USE_GPU)
    add_definitions(-DTE_USE_GPU)
endif(USE_GPU)

#add_definitions(-DNDEBUG)
#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=thread -fPIE -g")
#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=thread -fPIE -g")

set(FRONT_END
    main.cpp
)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/extern/glm
)

add_executable(TerrainEnginert ${FRONT_END})


target_link_libraries(TerrainEnginert
    TerrainEngine
    TaskEngine
    ${CMAKE_THREAD_LIBS_INIT}
)

## Add Lib
add_subdirectory(core/RenderEngine)
add_subdirectory(core/NetworkEngine)
add_subdirectory(core/TerrainEngine)
add_subdirectory(core/TaskEngine)
add_subdirectory(extern/glm)
#add_subdirectory(${CMAKE_SOURCE_DIR}/extern/glm ./glm)