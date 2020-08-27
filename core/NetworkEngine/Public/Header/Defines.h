#pragma once

// clang-format off
#if defined(_MSC_VER)
    #ifdef __NET_ENGINE_EXPORTS__
        #define NET_ENGINE_API __declspec(dllexport)
    #else
        #define NET_ENGINE_API __declspec(dllimport)
    #endif
#elif defined(__GNUC__)
    #ifdef __NET_ENGINE_EXPORTS__
        #define NET_ENGINE_API __attribute__((visibility("default")))
    #else
        #define NET_ENGINE_API
    #endif
#else
    #define NET_ENGINE_API
#endif
// clang-format on

#define NET_ENGINE_NAMESPACE TerrainEngine
#define NT_NS_OPEN                                                                                                     \
    namespace NET_ENGINE_NAMESPACE                                                                                     \
    {
#define NT_NS_CLOSE }