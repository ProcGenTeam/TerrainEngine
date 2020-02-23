#pragma once
#include <cstdint>

enum class EMemoryUseTypes : uint32_t
{
    AllMemory,
    LayerMemory,
    MethodMemory,
    TransientMemory,

    TOTAL_MEMORY_TYPES
};

enum class EOperationTypes : uint32_t
{
    Erode,
    Perlin,

    CreateLayer,
    DestoryLayer,

    MixLayers,
    MixLayersCustom,

    AddLayers,
    SubLayers,
    MulLayers,
    DivLayers,

    AddScalar,
    SubScalar,
    MulScalar,
    DivScalar,

    TOTAL_OPERATION_TYPES
};

struct FOperation
{
    EOperationTypes OpType;

    union
    {
        struct 
        {
            uint32_t u32Arg1;
            uint32_t u32Arg2;
        };
        uint64_t u64Arg1;
    };
    
    union
    {
        struct 
        {
            uint32_t u32Arg3;
            uint32_t u32Arg4;
        };
        uint64_t u64Arg2;
    };
};