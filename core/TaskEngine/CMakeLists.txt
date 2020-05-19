cmake_minimum_required(VERSION 3.7)
project(TaskEngine VERSION 0.0.1 DESCRIPTION "Task Scheduler Component")
set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

# Switch development flags
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

# Files
set(PrivateHeaders
    Private/Header/TaskEngine_Impl.h
    Private/Header/Connection.h
    
)

set(PrivateSources
    Private/Source/TaskEngine.cpp
    Private/Source/TaskEngine_Impl.cpp
)

set(PublicHeaders
    Public/Header/Defines.h
    Public/Header/TaskEngine.h
    Public/Header/ErrorCode.h
    Public/Header/Worker.h
    Public/Header/Task.h
)

# Add Shared Object
add_definitions(-D__TASK_ENGINE_EXPORTS__)
add_library(TaskEngine SHARED ${PublicHeaders} ${PrivateHeaders} ${PrivateSources})
set_target_properties(TaskEngine PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(TaskEngine PROPERTIES SOVERSION 1)
set_target_properties(TaskEngine PROPERTIES LINKER_LANGUAGE CXX)

link_directories(
    TaskEngine
)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

target_include_directories(TaskEngine PRIVATE
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(
    TaskEngine
    TerrainEngine
    NetworkEngine
    ${CMAKE_THREAD_LIBS_INIT}
)

add_subdirectory(Test)