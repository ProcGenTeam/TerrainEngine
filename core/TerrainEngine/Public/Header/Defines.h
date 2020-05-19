#pragma once

#if defined(_MSC_VER)
    #ifdef __TERRAIN_ENGINE_EXPORTS__
        #define TERRAIN_ENGINE_API __declspec(dllexport)
    #else
        #define TERRAIN_ENGINE_API __declspec(dllimport)
    #endif
#elif defined(__GNUC__)
    #ifdef __TERRAIN_ENGINE_EXPORTS__
        #define TERRAIN_ENGINE_API __attribute__((visibility("default")))
    #else
        #define TERRAIN_ENGINE_API
    #endif
#else
    #define TERRAIN_ENGINE_API
#endif

#define TERRAIN_ENGINE_NAMESPACE TerrainEngine
#define TE_NS_OPEN namespace TERRAIN_ENGINE_NAMESPACE {
#define TE_NS_CLOSE }

typedef float (*LayerMixer)(float, float);


#include "core/Common/Public/Header/Types.h"