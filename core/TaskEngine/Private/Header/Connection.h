#pragma once
#include <vector>
#include <cstdint>

struct FConnection
{
    uint64_t dummyInt;
    std::vector<char> key; // Used for keying
};