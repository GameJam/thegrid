#include "Random.h"

Random::Random()
{
    m_last = 0;
}

void Random::Seed(int seed)
{
    m_last = seed;
}

int Random::Generate(int min, int max)
{
    return (int)(Generate() * (max - min) + min + 0.5f);
}

float Random::Generate()
{
    return (((m_last = m_last * 214013L + 2531011L) >> 16) & 0x7fff) / (float)0x7fff;
}