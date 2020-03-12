#pragma once
#include <cstdint>

enum class ENetworkErrorCodes : uint32_t
{
    NoError,
    HostNotFound,
    HostKeyFailure,
    HostFailedVerification,
    HostDropped,

    Listen_InvalidPort,
    Listen_CreateError,
    Listen_AcceptError,

    TOTAL_ERROR_CODES
};