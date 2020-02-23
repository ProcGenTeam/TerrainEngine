cmake_minimum_required(VERSION 3.7)
project(LazyEval VERSION 0.0.1 DESCRIPTION "Terrain Generation Component")
set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

# Switch development flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

include_directories(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/core
)

add_executable(LazyEval TestLazyExe.cpp)


target_link_libraries(LazyEval
    TerrainEngine
    TaskEngine
    ${CMAKE_THREAD_LIBS_INIT}
)