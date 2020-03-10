cmake_minimum_required(VERSION 3.7)
project(NetworkEngine VERSION 0.0.1 DESCRIPTION "Network Component")
set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

# Switch development flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall --ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

# Files
set(PrivateHeaders
    Private/Header/NetworkEngine_Impl.h
    Private/Header/NetworkState.h
)

set(PrivateSources
    Private/Source/NetworkEngine_Impl.cpp
    Private/Source/NetworkEngine.cpp
)

set(PublicHeaders
    Public/Header/Defines.h
    Public/Header/NetworkEngine.h
    #Public/Header/Connection.h
    Public/Header/ErrorCode.h
)

# Add Shared Object
add_definitions(-D__TERRAIN_ENGINE_EXPORTS__)
add_library(NetworkEngine SHARED ${PublicHeaders} ${PrivateHeaders} ${PrivateSources})
set_target_properties(NetworkEngine PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(NetworkEngine PROPERTIES SOVERSION 1)
set_target_properties(NetworkEngine PROPERTIES LINKER_LANGUAGE CXX)

link_directories(
    NetworkEngine
)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

target_include_directories(NetworkEngine PRIVATE
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(
    NetworkEngine
    glm
    ${CMAKE_THREAD_LIBS_INIT}
)

#add_subdirectory(Test)