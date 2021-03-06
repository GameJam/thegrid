#include "Map.h"
#include "Random.h"
#include "Utility.h"
#include "Vec2.h"

#include <assert.h>
#include <math.h>
#include <malloc.h>
#include <memory.h>
#include <set>

const float pi = 3.1415926535f;

const int   _minTerminalDist = 20;
const float _stopMergeDistance = 25.0f;

const unsigned long kLineColor[] = 
{
    //0xFF231F20,
    0xFFDD83B6,
    0xFFF19526,
    0xFF949599,
    0xFFB31B64,
    0xFF39349A,
    0xFFE31F21,
    0xFF02A353,
    0xFF0098DF,
};

const int kNumLines = sizeof(kLineColor) / sizeof(unsigned long);


bool GetLineLineIntersection(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, float& ua, float& ub, Vec2& result)
{
    
    // Based on the algorithm here:
    // http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline2d/

    float d  = (p4.y - p3.y) * (p2.x - p1.x) - (p4.x - p3.x) * (p2.y - p1.y);

    if (d == 0.0f)
    {
        return false;
    }

    float na = (p4.x - p3.x) * (p1.y - p3.y) - (p4.y - p3.y) * (p1.x - p3.x);
    float nb = (p2.x - p1.x) * (p1.y - p3.y) - (p2.y - p1.y) * (p1.x - p3.x);
    
    ua = na / d;
    ub = nb / d;

    result = p1 + ua * (p2 - p1);
    return true;

}

bool GetLineSegmentLineSegmentIntersection(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4, Vec2& result)
{
    float ua, ub;
    if (GetLineLineIntersection(p1, p2, p3, p4, ua, ub, result))
    {
        // To test if the segments intersect, we need to check that ua and ub are
        // in the range 0 to 1.
        return ua > 0.0f && ub > 0.0f && ua < 1.0f && ub < 1.0f;
    }
    return false;
}


Map::Map()
{
    m_numStops = 0;
    m_numRails = 0;
    m_numRiverVertices = 0;
}

void Map::Generate(int xSize, int ySize, int seed)
{

    Random random;
    random.Seed(seed);

    const int terminalSpacing = 150;

    int xNumTiles = xSize / terminalSpacing;
    int yNumTiles = ySize / terminalSpacing;

    const int maxLines = 8;
    int line = 0;

    for (int i = 0; i < maxLines - 1; ++i)
    {

        int r = random.Generate(0, xNumTiles * 2 + yNumTiles * 2);

        int x = 0;
        int y = 0;

        if (r < xNumTiles)
        {
            x = r;
            y = 0;
        }
        else if (r < xNumTiles * 2)
        {
            x = r - xNumTiles;
            y = yNumTiles - 1;
        }
        else if (r < xNumTiles * 2 + yNumTiles)
        {
            x = 0;
            y = r - xNumTiles * 2;
        }
        else
        {
            x = xNumTiles - 1;
            y = r - (xNumTiles * 2 + yNumTiles);
        }

        if (x >= xNumTiles) x = xNumTiles - 1;
        if (y >= yNumTiles) y = yNumTiles - 1;

        assert(x >= 0);
        assert(y >= 0);

        Vec2 point;
        point.x = (float)(x * terminalSpacing + random.Generate(_minTerminalDist, terminalSpacing - _minTerminalDist));
        point.y = (float)(y * terminalSpacing + random.Generate(_minTerminalDist, terminalSpacing - _minTerminalDist));
        AddStop(point, line, true);
        ++line;

    }

 
    int numTerminals = m_numStops;

    for (int i = 0; i < numTerminals; ++i)
    {
        GenerateLine(xSize, ySize, i, random);
    }

   // Create an additional line that circles the center of the city.

    int xNumSteps = 2;
    int yNumSteps = 2;
    int lastStop = -1;
    int firstStop = -1;

    for (int i = 0; i < 16; ++i)
    {

        float x = cosf((i * 2 * 3.14159265f) / 15) * 2.0f;
        float y = sinf((i * 2 * 3.14159265f) / 15) * 1.5f;

        x += xNumTiles / 2;
        y += yNumTiles / 2;

        Vec2 point((x + 0.5f) * terminalSpacing, (y + 0.5f) * terminalSpacing);

        point.x = floorf(point.x / 50.0f) * 50.0f;
        point.y = floorf(point.y / 50.0f) * 50.0f;

        int stop = MergeStop(point, line, 50.0f);

        if (lastStop != -1)
        {
            Connect(lastStop, stop, line);
        }
        else
        {
            firstStop = stop;
        }
        lastStop = stop;
        
    }
    Connect(lastStop, firstStop, line);


    // Initialize the stop children.
    for (int i = 0; i < m_numRails; ++i)
    {
        const Rail& rail = m_rail[i];
        Stop& stop1 = m_stop[rail.stop1];
        Stop& stop2 = m_stop[rail.stop2];
        stop1.children.push_back(rail.stop2);
        stop2.children.push_back(rail.stop1);
    }

    /*
    // Interative straighen out the lines.

    const int numPasses = 100;
    for (int pass = 0; pass < numPasses; ++pass)
    {
        for (int i = 0; i < m_numStops; ++i)
        {
           Stop& stop = m_stop[i];
           StraightenStop(stop);
        }
    }
    */

    // Place the structures.

    const int numBanks  = 2;
    const int numPolice = 3;
    const int numTowers = 4;

    PlaceStructures(StructureType_Bank,   numBanks, random);
    PlaceStructures(StructureType_Police, numPolice, random);
    PlaceStructures(StructureType_Tower,  numTowers, random);

}

void Map::PlaceStructures(StructureType structureType, int number, Random& random)
{
    for (int i = 0; i < number; ++i)
    {
        int offset = random.Generate(0, m_numStops);
        for (int j = 0; j < m_numStops; ++j)
        {
            int stopIndex = (j + offset) % m_numStops;
            Stop& stop = m_stop[stopIndex];
            if (stop.structureType == StructureType_None)
            {
                stop.structureType = structureType;
                break;
            }
        }
    }
}

void Map::StraightenStop(Stop& stop)
{
    if (stop.children.size() == 2)
    {

        int child1 = stop.children[0];
        int child2 = stop.children[1];

        const Stop& stop1 = m_stop[child1];
        const Stop& stop2 = m_stop[child2];

        Vec2 e1 = stop1.point - stop.point;
        Vec2 e2 = stop2.point - stop.point;

        e1.Normalize();
        e2.Normalize();

        float angle = DotProduct( e1, e2 );

        if (angle < 0.01f)
        {
            // Make it into a straight line.
            Vec2 d = stop2.point - stop1.point;
            d.Normalize();
            Vec2 p = stop1.point + DotProduct(d, stop.point - stop1.point) * d;
            stop.point = p;
        }

    }
}

void Map::Connect(int stop1, int stop2, int line)
{
    assert(line >= 0);
    
    const Stop& s1 = m_stop[stop1];
    const Stop& s2 = m_stop[stop2];

    // Check if we are crossing an existing rail.

    for (int i = 0; i < m_numRails; ++i)
    {
        Rail& rail = m_rail[i];
        const Stop& s3 = m_stop[ rail.stop1 ];
        const Stop& s4 = m_stop[ rail.stop2 ];
        Vec2 mid;
        if (rail.stop1 != stop1 && rail.stop2 != stop1 &&
            rail.stop1 != stop2 && rail.stop2 != stop2 &&
            GetLineSegmentLineSegmentIntersection(s1.point, s2.point, s3.point, s4.point, mid))
        {
            int midStop = MergeStop(mid, -1, _stopMergeDistance);

            assert(m_numRails < s_maxRails);
            m_rail[m_numRails].stop1 = midStop;
            m_rail[m_numRails].stop2 = rail.stop2;
            m_rail[m_numRails].line  = rail.line;
            EnforceRailConstraint(m_rail[m_numRails]);
            ++m_numRails;
            
            rail.stop2 = midStop;
            EnforceRailConstraint(rail);

            Connect(stop1, midStop, line);
            Connect(midStop, stop2, line);
            return;
        }
    }

    assert(m_numRails < s_maxRails);
    m_rail[m_numRails].stop1 = stop1;
    m_rail[m_numRails].stop2 = stop2;
    EnforceRailConstraint(m_rail[m_numRails]);
    m_rail[m_numRails].line  = line;
    ++m_numRails;
}

int Map::AddStop(const Vec2& point, int line, bool terminal)
{
    assert(m_numStops < s_maxStops);
    m_stop[m_numStops].point            = point;
    m_stop[m_numStops].line             = line;
    m_stop[m_numStops].terminal         = terminal;
    m_stop[m_numStops].structureType    = StructureType_None;
    ++m_numStops;
    return m_numStops - 1;
}

int Map::MergeStop(const Vec2& point, int line, float distance)
{
    float distance2 = distance * distance;
    for (int i = 0; i < m_numStops; ++i)
    {
        Vec2 d = m_stop[i].point - point;
        if (d.x * d.x + d.y * d.y < distance2 && !m_stop[i].terminal)
        {
            m_stop[i].line = -1;
            return i;
        }
    }
    return AddStop(point, line);
}


void Map::GenerateLine(int xSize, int ySize, int stopIndex, Random& random)
{

    // Lines go along horizontal, vertical and 45 degree angles.
    const int numDirs = 8;
    
    const Stop& stop = m_stop[stopIndex];

    bool xPos = stop.point.x < xSize / 2;
    bool yPos = stop.point.y < ySize / 2;

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

    Vec2 point = stop.point;
    int dir  = random.Generate(minDir, maxDir);
    int line = stop.line;
    
    int stepSize = random.Generate(50, 100);

    for (int i = 0; i < 20; ++i)
    {

        if (random.Generate(0, 100) > 25)
        {   
            dir = random.Generate(minDir, maxDir);
        }

        float xStep = cosf(dir * 2.0f * pi / numDirs) * stepSize;
        float yStep = sinf(dir * 2.0f * pi / numDirs) * stepSize;   

        point.x += xStep;
        point.y += yStep;

        if (point.x > xSize - _minTerminalDist || point.x < _minTerminalDist ||
            point.y > ySize - _minTerminalDist || point.y < _minTerminalDist)
        {
            break;
        }

        int newStopIndex = MergeStop(point, stop.line, _stopMergeDistance);
        point = m_stop[newStopIndex].point;

        Connect(stopIndex, newStopIndex, line);
        stopIndex = newStopIndex;

    }

    m_stop[stopIndex].terminal = true;

}

void Map::EnforceRailConstraint(Rail& rail) const
{
    if (rail.stop1 > rail.stop2)
    {
        Swap(rail.stop1, rail.stop2);
    }
}

int Map::GetStopForPoint(const Vec2& point)
{
    float distance = 10.0f;
    float d2 = distance * distance;
    for (int i = 0; i < m_numStops; ++i)
    {
        Vec2 offset = m_stop[i].point - point;
        if (DotProduct(offset, offset) < d2)
        {
            return i;
        }
    }
    return -1;
}

int Map::GetNearestStopForPoint(const Vec2& point)
{
    int closestStop = -1;
    float minDistanceSquared;

    for (int i = 0; i < m_numStops; ++i)
    {
        Vec2 offset = m_stop[i].point - point;
        float distanceSquared = DotProduct(offset, offset);
        
        if (closestStop == -1 || distanceSquared < minDistanceSquared)
        {
            closestStop = i;
            minDistanceSquared = distanceSquared;
        }
    }

    return closestStop;
}

int Map::GetLineBetween(int stopA, int stopB)
{
    for (int i = 0 ; i < m_numRails; ++i)
    {
        if ((m_rail[i].stop1 == stopA && m_rail[i].stop2 == stopB) ||
            (m_rail[i].stop1 == stopB && m_rail[i].stop2 == stopA))
        {
            return m_rail[i].line;
        }
    }

    return -1;
}

unsigned long Map::GetLineColor(int line)
{

    if (line < 0)
    {
        return 0xff7f7f7f;
    }

    return kLineColor[line % kNumLines];

}

int Map::GetPath(int stopA, int stopB, int path[])
{
    struct Node
    {
        int distance;
        bool visited;
        int next;
    };

    if (stopA == stopB)
    {
        path[0] = stopA;
        return 1;
    }

    Node nodes[s_maxStops];

    int unvisited[s_maxStops];
    int numUnvisited = 0;

    // Setup
    for (int i = 0; i < m_numStops; ++i)
    {
        if (i == stopB)
        {
            nodes[i].distance = 0;
        }
        else
        {
            nodes[i].distance = -1;
            unvisited[numUnvisited] = i;
            ++numUnvisited;
        }
        nodes[i].visited = false;
        nodes[i].next = -1;        
    }

    int current = stopB;

    // Dijkstra's algorithm
    while(true)
    {
        int distance = nodes[current].distance + 1;
        for (size_t i = 0; i < m_stop[current].children.size(); ++i)
        {
            int neighbor = m_stop[current].children[i];
            if (!nodes[neighbor].visited && (nodes[neighbor].distance == -1 || distance < nodes[neighbor].distance))
            {
                nodes[neighbor].distance = distance;
                nodes[neighbor].next = current;
            }
        }
        nodes[current].visited = true;

        // Find nearest unvisited
        int nearestIndex = -1;
        int nearestDistance;
        for (int i = 1; i < numUnvisited; ++i)
        {
            distance = nodes[unvisited[i]].distance;
            if (distance != -1 && (nearestIndex == -1 || distance < nearestDistance))
            {
                nearestIndex = i;
                nearestDistance = distance;
            }
        }
        
        if (nearestIndex == -1)
        {
            // No path!
            return 0;
        }

        current = unvisited[nearestIndex];
        if (current == stopA)
        {
            break;
        }

        unvisited[nearestIndex] = unvisited[numUnvisited-1];
        --numUnvisited;
    }

    // Build path
    int pathLength = 0;
    while (current != stopB)
    {
        path[pathLength] = current;
        ++pathLength;
        current = nodes[current].next;
        assert(current != -1);
    }
    path[pathLength] = stopB;
    ++pathLength;

    return pathLength;

}