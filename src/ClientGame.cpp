#include "ClientGame.h"
#include "Log.h"

#include <math.h>
#include <assert.h>

const int gridSpacing = 150;
const int xMapSize = gridSpacing * 9;
const int yMapSize = gridSpacing * 6;

static void DrawCircle(const Vec2& point, int radius)
{
    const int numSides = 16;
    glBegin(GL_TRIANGLE_FAN);
    for (int j = 0; j < numSides; ++j)
    {
        float x = point.x + cosf((2.0f * j * 3.14159265f) / (numSides - 1)) * radius;
        float y = point.y + sinf((2.0f * j * 3.14159265f) / (numSides - 1)) * radius;
        glVertex2f(x, y);
    }
    glEnd();
}

ClientGame::ClientGame(int xSize, int ySize) : m_host(1)
{
    m_mapScale  = 1;
    m_xSize     = xSize;
    m_ySize     = ySize;
    m_state     = State_Idle;
    m_blipX     = 0;
    m_blipY     = 0;
    m_serverId  = -1;

    CenterMap(xMapSize / 2, yMapSize / 2);
}

void ClientGame::LoadResources()
{

    struct TextureLoad
    {
        Texture*    texture;
        const char* fileName;
    };

    TextureLoad load[] = 
        { 
            { &m_agentTexture, "assets/agent.png"         },
        };

    int numTextures = sizeof(load) / sizeof(TextureLoad);
    for (int i = 0; i < numTextures; ++i)
    {   
        Texture_Load(*load[i].texture, load[i].fileName);
    }

    Font_Load(m_font, "assets/font.csv");

    m_map.Generate(xMapSize, yMapSize, 3);

}

void ClientGame::Render() const
{

    const unsigned long lineColor[] = 
        {
            0xFF231F20,
            0xFFF19526,
            0xFF949599,
            0xFFB31B64,
            0xFF39349A,
            0xFFE31F21,
            0xFF02A353,
        };

    int numLines = sizeof(lineColor) / sizeof(unsigned long);

    glClearColor( 0.97f, 0.96f, 0.89f, 0.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glDisable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(m_mapX, m_mapX + m_xSize * m_mapScale, m_mapY + m_ySize * m_mapScale, m_mapY);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const int outerBorder = 35;

    // Draw the background.
    glColor(0xFFFFFFFF);
    glBegin(GL_QUADS);
    glVertex2i(-outerBorder, -outerBorder);
    glVertex2i(xMapSize + outerBorder, -outerBorder);
    glVertex2i(xMapSize + outerBorder, yMapSize + outerBorder);
    glVertex2i(-outerBorder, yMapSize + outerBorder);
    glEnd();

    // Outer thick border of the map
    glColor(0xFF7FD6F2);
    glLineWidth(2);
    glBegin(GL_LINE_LOOP);
    glVertex2i(-outerBorder, -outerBorder);
    glVertex2i(xMapSize + outerBorder, -outerBorder);
    glVertex2i(xMapSize + outerBorder, yMapSize + outerBorder);
    glVertex2i(-outerBorder, yMapSize + outerBorder);
    glEnd();

    // Grid
    glColor(0xFF7FD6F2);
    glLineWidth(1);
    glBegin(GL_LINES);
    for (int x = 0; x < xMapSize / gridSpacing; ++x)
    {
        glVertex2i(x * gridSpacing, 0);
        glVertex2i(x * gridSpacing, yMapSize);
    }
    glVertex2i(xMapSize, 0);
    glVertex2i(xMapSize, yMapSize);
    for (int y = 0; y < yMapSize / gridSpacing; ++y)
    {
        glVertex2i(0, y * gridSpacing);
        glVertex2i(xMapSize, y * gridSpacing);
    }
    glVertex2i(0, yMapSize);
    glVertex2i(xMapSize, yMapSize);
    glEnd();

    // Rails.
    glLineWidth(8.0f / m_mapScale);
    glBegin(GL_LINES);
    for (int i = 0; i < m_map.GetNumRails(); ++i)
    {
        const Rail& rail  = m_map.GetRail(i);
        const Stop& stop1 = m_map.GetStop(rail.stop1);
        const Stop& stop2 = m_map.GetStop(rail.stop2);
        assert(rail.line >= 0);
        glColor( lineColor[rail.line % numLines] );
        glVertex(stop1.point);
        glVertex(stop2.point);
    }
    glEnd();

    // Stops.
    for (int i = 0; i < m_map.GetNumStops(); ++i)
    {
        const Stop& stop = m_map.GetStop(i);
        if (stop.line == -1)
        {
            glColor( 0xFF000000 );
            DrawCircle(stop.point, 8);
            glColor( 0xFFFFFFFF );
            DrawCircle(stop.point, 6);
        }
        else
        {
            glColor( lineColor[stop.line % numLines] );
            DrawCircle(stop.point, 8);
        }
    }

    glEnable(GL_TEXTURE_2D);    

    glColor(0xFFFFFFFF);

    for (size_t i = 0; i < m_testState.m_test.size(); ++i)
    {
        Render_DrawSprite(m_agentTexture, m_testState.m_test[i].x, m_testState.m_test[i].y);
    }

    Font_BeginDrawing(m_font, m_xSize, m_ySize);

    glColor(0xFF000000);
    Font_DrawText("Hello!", 10, 10);

    Font_EndDrawing();

}

void ClientGame::OnMouseDown(int x, int y, int button)
{
    if (button == 1)
    {
        // Toggle zoom on left click.
        if (m_mapScale == 1)
        {
            SetMapScale(2, x, y);
        }
        else
        {
            SetMapScale(1, x, y);
        }
    }

    if (button == 2)
    {
        int blipX, blipY;
        ScreenToWorld(x, y, blipX, blipY);

        Protocol::OrderPacket order;
        order.x = blipX;
        order.y = blipY;
        SendOrder(order);
    }

    if (button == 3)
    {
        // Pan with right mouse button.
        if (m_state == State_Idle)
        {
            m_state = State_Panning;
            m_stateX = x;
            m_stateY = y;
        }
    }

}

void ClientGame::OnMouseUp(int x, int y, int button)
{
    if (button == 3)
    {
        if (m_state == State_Panning)
        {
            m_state = State_Idle;
        }
    }
}

void ClientGame::OnMouseMove(int x, int y)
{
    if (m_state == State_Panning)
    {
        int xDelta = m_stateX - x;
        int yDelta = m_stateY - y;
        m_mapX += xDelta * m_mapScale;
        m_mapY += yDelta * m_mapScale;
        m_stateX = x;
        m_stateY = y;
    }
}

void ClientGame::ScreenToWorld(int xScreen, int yScreen, int& xWorld, int& yWorld) const
{
    xWorld = xScreen * m_mapScale + m_mapX;
    yWorld = yScreen * m_mapScale + m_mapY;
}

void ClientGame::SetMapScale(int scale, int xScreen, int yScreen)
{
    m_mapX = xScreen * m_mapScale + m_mapX - xScreen * scale;
    m_mapY = yScreen * m_mapScale + m_mapY - yScreen * scale;
    m_mapScale = scale;
}

void ClientGame::CenterMap(int xWorld, int yWorld)
{
    m_mapX = xWorld - (m_xSize / 2) * m_mapScale;
    m_mapY = yWorld - (m_ySize / 2) * m_mapScale;
}

void ClientGame::Connect(const char* hostName, int port)
{
    m_host.Connect(hostName, port);
}

void ClientGame::Update()
{
    m_host.Service(this);

    if (m_testState.m_id != -1)
    {
        m_blipX = m_testState.m_test[m_testState.m_id].x;
        m_blipY = m_testState.m_test[m_testState.m_id].y;
    }
}

void ClientGame::OnConnect(int peerId)
{
    m_serverId = peerId;    
}

void ClientGame::OnDisconnect(int peerId)
{
    m_serverId = -1;
}

void ClientGame::OnPacket(int peerId, int channel, void* data, size_t size)
{
    if (peerId == m_serverId)
    {
        char* byteData = static_cast<char*>(data);
        Protocol::PacketType packetType = static_cast<Protocol::PacketType>(*byteData);
        
        switch (packetType)
        {
        case Protocol::PacketType_State:
            {
                Protocol::StatePacket* packet = static_cast<Protocol::StatePacket*>(data);
                m_testState.Deserialize(packet->data);
            }
            break;

        default:
            LogDebug("Unrecognized packet: %i", packetType);
        }        
    }
}

void ClientGame::SendOrder(Protocol::OrderPacket& order)
{

    if (m_serverId == -1)
    {
        LogError("SendOrder: not connected to server");
        return;
    }

    order.packetType = Protocol::PacketType_Order;
    m_host.SendPacket(m_serverId, 0, &order, sizeof(Protocol::OrderPacket));

}
