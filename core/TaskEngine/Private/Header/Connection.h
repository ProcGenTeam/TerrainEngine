#pragma once
#include <cstdint>
#include <vector>

struct FConnection
{
    uint64_t dummyInt;
    std::vector<char> key; // Used for keying
};