cmake_minimum_required(VERSION 3.7)
project(RenderEngine VERSION 0.0.1 DESCRIPTION "Renderer Component")
set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# Switch development flags
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall --ggdb")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

# Files
set(PrivateHeaders
    Private/Header/RenderEngine_Impl.h
    Private/Header/RenderHelpers.h

)

set(PrivateSources
    Private/Source/RenderEngine_Impl.cpp
    Private/Source/RenderEngine.cpp
    Private/Source/RenderHelpers.cpp
)

set(PublicHeaders
    Public/Header/RenderEngine.h
    Public/Header/Common.h
)

# Add Shared Object
add_definitions(-D__TERRAIN_ENGINE_EXPORTS__)
add_library(RenderEngine SHARED ${PublicHeaders} ${PrivateHeaders} ${PrivateSources})
set_target_properties(RenderEngine PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(RenderEngine PROPERTIES SOVERSION 1)
set_target_properties(RenderEngine PROPERTIES LINKER_LANGUAGE CXX)

link_directories(
    RenderEngine
)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/extern/glm
)

target_include_directories(RenderEngine PRIVATE
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(
    RenderEngine
    glm
    ${CMAKE_THREAD_LIBS_INIT}
)

#add_subdirectory(${CMAKE_SOURCE_DIR}/extern/glm ./glm)