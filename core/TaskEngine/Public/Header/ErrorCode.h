#pragma once

#include <bits/stdint-uintn.h>
enum class ETaskErrorCodes : uint32_t
{
    HostNotFound,
    HostKeyFailure,

    TOTAL_ERROR_CODES
};