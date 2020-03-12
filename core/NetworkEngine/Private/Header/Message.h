#pragma once
#include "NetworkEngine/Public/Header/ErrorCode.h"
#include <cstdint>

enum class EThreadType : uint32_t
{
    Dispatch,
    Listener,
    
    Commission,

    General,

    TOTAL_THREAD_TYPES
};

struct FNetMessage
{
    EThreadType ThreadType;
    ENetworkErrorCodes ErrorType;
    union
    {
        uint64_t u64_Arg;
        struct
        {
            uint32_t u32_Arg1;
            uint32_t u32_Arg2;
        };
    };
};
