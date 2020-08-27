#pragma once
#include "Common/Public/Header/Error.h"
#include <cstdint>

enum class EThreadType : uint32_t
{
    Dispatch,
    Listener,

    Commission,

    General,

    Task_NetHandler,

    TOTAL_THREAD_TYPES
};

struct FMessage
{
    EThreadType ThreadType;
    EErrorCodes ErrorType;
    union {
        uint64_t u64_Arg;
        struct
        {
            uint32_t u32_Arg1;
            uint32_t u32_Arg2;
        };
    };
};
