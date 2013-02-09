#include "Map.h"
#include "Random.h"
#include "Utility.h"

#include <assert.h>
#include <math.h>

const float pi = 3.1415926535f;

Map::Map()
{
    m_numStops = 0;
    m_numRails = 0;
}

void Map::Generate(int xSize, int ySize, int seed)
{

    Random random;
    random.Seed(seed);

    const int terminalSpacing = 150;

    int xNumTiles = xSize / terminalSpacing;
    int yNumTiles = ySize / terminalSpacing;

    int line = 0;

    for (int x = 0; x < xNumTiles; ++x)
    {
        for (int y = 0; y < yNumTiles; ++y)
        {

            // Make terminals less likely towards the interior of the map

            const int maxProb  = 30;
            const int probStep = 15;
            const int minDist  = 20;

            int xDist = Min(x, xNumTiles - 1 - x);
            int yDist = Min(y, yNumTiles - 1 - y);
            int prob  = Max(xDist, yDist) * probStep + maxProb;

            if (random.Generate(0, 100) > prob)
            {
                AddStop(x * terminalSpacing + random.Generate(minDist, terminalSpacing - minDist),
                        y * terminalSpacing + random.Generate(minDist, terminalSpacing - minDist),
                        line,
                        true);
                ++line;
            }

        }
    }

    int numTerminals = m_numStops;

    for (int i = 0; i < numTerminals; ++i)
    {
        GenerateLine(xSize, ySize, i, random);
    }
    
}

void Map::Connect(int stop1, int stop2, int line)
{
    assert(line >= 0);
    assert(m_numRails < s_maxRails);
    m_rail[m_numRails].stop1 = stop1;
    m_rail[m_numRails].stop2 = stop2;
    m_rail[m_numRails].line  = line;
    ++m_numRails;
}

int Map::AddStop(int x, int y, int line, bool terminal)
{
    assert(m_numStops < s_maxStops);
    m_stop[m_numStops].x = x;
    m_stop[m_numStops].y = y;
    m_stop[m_numStops].line = line;
    m_stop[m_numStops].terminal = terminal;
    ++m_numStops;
    return m_numStops - 1;
}

int Map::MergeStop(int x, int y, int line, int distance)
{
    int distance2 = distance * distance;
    for (int i = 0; i < m_numStops; ++i)
    {
        int dx = m_stop[i].x - x;
        int dy = m_stop[i].y - y;
        if (dx * dx + dy * dy < distance2 && !m_stop[i].terminal)
        {
            m_stop[i].line = -1;
            return i;
        }
    }
    return AddStop(x, y, line);
}


void Map::GenerateLine(int xSize, int ySize, int stopIndex, Random& random)
{

    // Lines go along horizontal, vertical and 45 degree angles.
    const int numDirs = 8;
    
    const Stop& stop = m_stop[stopIndex];

    bool xPos = stop.x < xSize / 2;
    bool yPos = stop.y < ySize / 2;

    int minDir, maxDir;

    if (xPos && yPos)
    {
        minDir = 0; maxDir = numDirs / 4;
    }
    else if (!xPos && yPos)
    {
        minDir = numDirs / 4; maxDir = numDirs / 2 - 1;
    }
    else if (!xPos && !yPos)
    {
        minDir = numDirs / 2; maxDir = (numDirs * 3) / 4 - 1;
    }
    else if (xPos && !yPos)
    {
        minDir = (numDirs * 3) / 4; maxDir = numDirs - 1;
    }

    int x = stop.x;
    int y = stop.y;
    int dir = random.Generate(minDir, maxDir);
    int line = stop.line;
    
    int stepSize = 50;
    const int mergeDistance = 25;

    for (int i = 0; i < 20; ++i)
    {

        if (random.Generate(0, 100) > 25)
        {   
            dir = random.Generate(minDir, maxDir);
        }

        int xStep = (int)(cosf(dir * 2.0f * pi / numDirs) * stepSize);
        int yStep = (int)(sinf(dir * 2.0f * pi / numDirs) * stepSize);   

        x += xStep;
        y += yStep;

        if (x > xSize || x < 0 || y > ySize || y < 0)
        {
            break;
        }

        int newStopIndex = MergeStop(x, y, stop.line, mergeDistance);

        x = m_stop[newStopIndex].x;
        y = m_stop[newStopIndex].y;

        assert(!m_stop[newStopIndex].terminal);

        Connect(stopIndex, newStopIndex, line);
        stopIndex = newStopIndex;

    }


}