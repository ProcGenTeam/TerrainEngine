#pragma once
#include <cstdint>

enum class EOperationTypes : uint32_t
{
    Erode,
    Perlin,

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
    
    uint32_t Arg3;
};