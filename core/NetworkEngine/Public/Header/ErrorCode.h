#pragma once
#include <cstdint>

enum class ENetworkErrorCodes : uint32_t
{
    HostNotFound,
    HostKeyFailure,
    HostFailedVerification,
    HostDropped,

    TOTAL_ERROR_CODES
};