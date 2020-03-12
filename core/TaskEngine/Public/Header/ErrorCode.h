#pragma once
#include <cstdint>

enum class ETaskErrorCodes : uint32_t
{
    NoError,
    HostNotFound,
    HostKeyFailure,

    NoEngines,

    TOTAL_ERROR_CODES
};