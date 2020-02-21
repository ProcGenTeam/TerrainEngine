#include <cstdint>
#include "TerrainEngine/Private/Header/PerlinNoise.h"
#include <algorithm>

inline double Lerp(double v0, double v1, double t)
{
    return (1 - t) * v0 + t * v1;
}

inline double Fade(double in)
{
    return in * in * in * (in * (in * 6 - 15) + 10);
}

inline double Grad(int grad, double x, double y, double z)
{
    int gLow = grad & 15;

    double u = gLow < 8 ? x : y;
    double v = gLow < 4 ? y : gLow == 12 || gLow == 14 ? x : z;

    return ((gLow & 1) == 0 ? u : -u) + ((gLow & 2) == 0 ? v : -v);
}

CPerlinNoiseGen::CPerlinNoiseGen(uint32_t uSeed)
{
    /// Resize the vector
    this->m_vParam.resize(256);

    /// Initial with values from 0 to 255
    iota(this->m_vParam.begin(), this->m_vParam.end(), 0);

    /// Create Random generator
    //std::default_random_engine rGen(seed);
    std::mt19937_64 rGen(uSeed);

    /// Shuffle vector using rGen engine
    shuffle(this->m_vParam.begin(), this->m_vParam.end(), rGen);

    /// Dup Vector
    this->m_vParam.insert(this->m_vParam.end(),this->m_vParam.begin(), this->m_vParam.end());
}

double CPerlinNoiseGen::Generate(double xCoord, double yCoord, double zCoord)
{
    auto ixCoord = int(floor(xCoord)) & 0xFF;
    auto iyCoord = int(floor(yCoord)) & 0xFF;
    auto izCoord = int(floor(zCoord)) & 0xFF;

    xCoord -= floor(xCoord);
    yCoord -= floor(yCoord);
    zCoord -= floor(zCoord);

    auto x = Fade(xCoord);
    auto y = Fade(yCoord);
    auto z = Fade(zCoord);

    auto pA = this->m_vParam[ixCoord] + iyCoord;
    auto pAa = this->m_vParam[pA] + izCoord;
    auto pAb = this->m_vParam[pA + 1] + izCoord;

    auto pB = this->m_vParam[ixCoord + 1] + iyCoord;
    auto pBa = this->m_vParam[pB] + izCoord;
    auto pBb = this->m_vParam[pB + 1] + izCoord;

    double res = Lerp(
        Lerp(
            Lerp(
                Grad(this->m_vParam[pAa], xCoord, yCoord, zCoord),
                Grad(this->m_vParam[pBa], xCoord - 1, yCoord, zCoord),
                x),
            Lerp(
                Grad(this->m_vParam[pAb], xCoord, yCoord - 1, zCoord),
                Grad(this->m_vParam[pBb], xCoord - 1, yCoord - 1, zCoord),
                x),
            y),
        Lerp(
            Lerp(
                Grad(this->m_vParam[pAa + 1], xCoord, yCoord, zCoord - 1),
                Grad(this->m_vParam[pBa + 1], xCoord - 1, yCoord, zCoord - 1),
                x),
            Lerp(
                Grad(this->m_vParam[pAb + 1], xCoord, yCoord - 1, zCoord - 1),
                Grad(this->m_vParam[pBb + 1], xCoord - 1, yCoord - 1, zCoord - 1),
                x),
            y),
        z);
    return (res + 1.0) / 2.0;
}

CPerlinNoiseGen::~CPerlinNoiseGen()
{
    
}