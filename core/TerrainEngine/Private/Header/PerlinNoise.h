#include <cstdint>
#include <random>
#include <vector>

class CPerlinNoiseGen
{
private:
    std::vector<int> m_vParam;

public:
    CPerlinNoiseGen(uint32_t uSeed);
    ~CPerlinNoiseGen();

    virtual double Generate(double xCoord, double yCoord, double zCoord);
    virtual double Fractal(double xCoord, double yCoord, double zCoord, uint32_t uLevels);
};