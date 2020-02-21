#include <cstdint>
#include <vector>
#include <random>

class CPerlinNoiseGen
{
    private:
        std::vector<int> m_vParam;

    public:
        CPerlinNoiseGen(uint32_t uSeed);
        ~CPerlinNoiseGen();

        virtual double Generate(double xCoord, double yCoord, double zCoord);
};