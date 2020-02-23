cmake_minimum_required(VERSION 3.7)
project(Tests VERSION 0.0.1 DESCRIPTION "Terrain Generation Component")
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(Functionality)
add_subdirectory(NoExecution)
add_subdirectory(Lazy)
