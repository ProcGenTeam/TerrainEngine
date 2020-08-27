#pragma once

// clang-format off
#if defined(_MSC_VER)
    #ifdef __TASK_ENGINE_EXPORTS__
        #define TASK_ENGINE_API __declspec(dllexport)
    #else
        #define TASK_ENGINE_API __declspec(dllimport)
    #endif
#elif defined(__GNUC__)
    #ifdef __TASK_ENGINE_EXPORTS__
        #define TASK_ENGINE_API __attribute__((visibility("default")))
    #else
        #define TASK_ENGINE_API
    #endif
#else
    #define TASK_ENGINE_API
#endif
// clang-format on

#define TASK_ENGINE_NAMESPACE TerrainEngine
#define TS_NS_OPEN                                                                                                     \
    namespace TASK_ENGINE_NAMESPACE                                                                                    \
    {
#define TS_NS_CLOSE }