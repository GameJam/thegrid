#ifndef GAME_MAP_H
#define GAME_MAP_H

#include "Vec2.h"
#include <vector>

class Random;

enum StructureType
{
    StructureType_None,
    StructureType_Bank,
    StructureType_Police,
    StructureType_Tower,
};

struct Stop
{
    Vec2                point;
    int                 line;       // -1 means a hub
    bool                terminal;   // End of the line buddy
    std::vector<int>    children;
    StructureType       structureType;
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

    int GetStopForPoint(const Vec2& point);

private:

    int  AddStop(const Vec2& point, int line, bool terminal = false);
    int  MergeStop(const Vec2& point, int line, float distance);

    void Connect(int stop1, int stop2, int line);

    void GenerateLine(int xSize, int ySize, int stopIndex, Random& random);

    void EnforceRailConstraint(Rail& rail) const;
    void StraightenStop(Stop& stop);

    void PlaceStructures(StructureType structureType, int number, Random& random);

private:

    static const int s_maxStops = 500;
    static const int s_maxRails = 1500;
    static const int s_maxRiverVertices = 100;

    int     m_numStops;
    Stop    m_stop[s_maxStops];

    int     m_numRails;
    Rail    m_rail[s_maxRails];

    int     m_numRiverVertices;
    Vec2    m_riverVertex[s_maxRiverVertices];

};

#endif