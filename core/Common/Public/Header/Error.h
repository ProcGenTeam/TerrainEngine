#pragma once
#include <cstdint>

enum class EErrorCodes : uint32_t
{
    NoError,

    HostNotFound,
    HostKeyFailure,
    HostFailedVerification,
    HostDropped,

    Task_NoEngines,

    Listen_InvalidPort,
    Listen_CreateError,
    Listen_AcceptError,

    Connect_AlreadyConnectedError,
    Connect_GeneralFailure,

    Proc_InsufficientMemory,

    TOTAL_ERROR_CODES
};

#include <string>
constexpr std::string_view EnglishDebugArray[12] = {"NoError",
                                                    "HostNotFound",
                                                    "HostKeyFailure",
                                                    "HostFailedVerification",
                                                    "HostDropped",
                                                    "Task_NoEngines",
                                                    "Listen_InvalidPort",
                                                    "Listen_CreateError",
                                                    "Listen_AcceptError",
                                                    "Connect_AlreadyConnectedError",
                                                    "Connect_GeneralFailure",
                                                    "TOTAL_ERROR_CODES"};

inline std::string_view GetName_en_gb(EErrorCodes error)
{
    return EnglishDebugArray[static_cast<std::underlying_type<EErrorCodes>::type>(error)];
}
