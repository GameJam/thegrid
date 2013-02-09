#ifndef GAME_RANDOM_H
#define GAME_RANDOM_H

class Random
{

public:

    Random();

    void Seed(int seed);

    int  Generate(int min, int max);

private:

    float Generate();

private:

    int m_last;

};

#endif