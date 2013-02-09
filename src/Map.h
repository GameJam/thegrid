#ifndef GAME_MAP_H
#define GAME_MAP_H

class Random;

struct Stop
{
    int     x;
    int     y;
    int     line; // -1 means a hub
    bool    terminal; // End of the line buddy
};

struct Rail
{
    int stop1;
    int stop2;
    int line;
};

class Map
{

public:

    Map();

    void Generate(int xSize, int ySize, int seed);

    int         GetNumStops() const { return m_numStops; }
    const Stop& GetStop(int i) const { return m_stop[i]; }

    int         GetNumRails() const { return m_numRails; }
    const Rail& GetRail(int i) const { return m_rail[i]; }

private:

    int  AddStop(int x, int y, int line, bool terminal = false);
    int  MergeStop(int x, int y, int line, int distance);

    void Connect(int stop1, int stop2, int line);

    void GenerateLine(int xSize, int ySize, int stopIndex, Random& random);

private:

    static const int s_maxStops = 500;
    static const int s_maxRails = 1500;

    int     m_numStops;
    Stop    m_stop[s_maxStops];

    int     m_numRails;
    Rail    m_rail[s_maxRails];

};

#endif