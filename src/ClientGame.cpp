#include "ClientGame.h"
#include "Log.h"
#include "Entity.h"

#include <math.h>
#include <assert.h>

const int gridSpacing       = 150;
const int xMapSize          = gridSpacing * 9;
const int yMapSize          = gridSpacing * 6;
const int yStatusBarSize    = 140;

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

ClientGame::ClientGame(int xSize, int ySize) 
    : m_host(1),
      m_testState(&m_entityTypes)
{

    InitializeEntityTypes(m_entityTypes);

    m_mapScale  = 1;
    m_xSize     = xSize;
    m_ySize     = ySize;
    m_state     = State_Idle;
    m_blipX     = 0;
    m_blipY     = 0;
    m_serverId  = -1;
    m_hoverStop = -1;

    CenterMap(xMapSize / 2, yMapSize / 2);

    UpdateActiveButtons();

    m_music = BASS_StreamCreateFile(FALSE, "assets/slow march.mp3", 0, 0, BASS_SAMPLE_LOOP);
    BASS_ChannelPlay(m_music, TRUE);

}

ClientGame::~ClientGame()
{
    if (m_music != NULL)
    {
        BASS_StreamFree(m_music);
    }
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
            { &m_agentTexture,                          "assets/agent.png"              },
            { &m_buttonTexture[ButtonId_Infiltrate],    "assets/action_infiltrate.png"  },
            { &m_buttonTexture[ButtonId_Capture],       "assets/action_capture.png"     },
            { &m_buttonTexture[ButtonId_Stakeout],      "assets/action_stakeout.png"    },
            { &m_buttonTexture[ButtonId_Hack],          "assets/action_hack.png"        },
            { &m_buttonTexture[ButtonId_Intel],         "assets/action_drop.png"        },
            { &m_buttonShadowTexture,                   "assets/button_shadow.png"      },
        };

    int numTextures = sizeof(load) / sizeof(TextureLoad);
    for (int i = 0; i < numTextures; ++i)
    {   
        Texture_Load(*load[i].texture, load[i].fileName);
    }

    Font_Load(m_font, "assets/font.csv");

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

    glClearColor( 0.97f * 0.9f, 0.96f * 0.9f, 0.89f * 0.9f, 0.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    //glEnable(GL_MULTISAMPLE);

    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST );
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST );
 
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);

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

        float inflate = 0.0f;
        if (i == m_hoverStop)
        {
            inflate = 5.0f;
        }
        if (stop.line == -1)
        {
            glColor( 0xFF000000 );
            DrawCircle(stop.point, 8 + inflate);
            glColor( 0xFFFFFFFF );
            DrawCircle(stop.point, 6 + inflate);
        }
        else
        {
            glColor( lineColor[stop.line % numLines] );
            DrawCircle(stop.point, 8 + inflate);
        }
    }

    // Draw the legend of the grid.
    Font_BeginDrawing(m_font);
    glColor(0xFF7FD6F2);
    int fontHeight = Font_GetTextHeight(m_font);
    for (int x = 0; x < xMapSize / gridSpacing; ++x)
    {
        char buffer[32];
        sprintf(buffer, "%d", x);
        int textWidth = Font_GetTextWidth(m_font, buffer);
        Font_DrawText(buffer, x * gridSpacing + gridSpacing / 2 - textWidth / 2, -outerBorder + 5);
        Font_DrawText(buffer, x * gridSpacing + gridSpacing / 2 - textWidth / 2, yMapSize + 5);
    }
    for (int y = 0; y < yMapSize / gridSpacing; ++y)
    {
        char buffer[32];
        sprintf(buffer, "%c", 'A' + y);
        Font_DrawText(buffer, -outerBorder + 10, y * gridSpacing + gridSpacing / 2 - fontHeight / 2);
        Font_DrawText(buffer, xMapSize + 10, y * gridSpacing + gridSpacing / 2 - fontHeight / 2);
    }
    Font_EndDrawing();


    glEnable(GL_TEXTURE_2D);    
    glColor(0xFFFFFFFF);

    for (int i = 0; i < m_testState.GetNumEntities(); ++i)
    {
        const Entity* entity = m_testState.GetEntity(i);
        
        // HACK!
        const TestEntity* testEntity = static_cast<const TestEntity*>(entity);
        Render_DrawSprite(m_agentTexture, testEntity->x, testEntity->y);
    }

    // Draw the UI.

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, m_xSize, m_ySize, 0);


    /*
    glDisable(GL_TEXTURE_2D);
    glColor(0x80000000);
    glBegin(GL_QUADS);
    glVertex2i(m_xSize - xTextureSize - 40, 0);
    glVertex2i(m_xSize, 0);
    glVertex2i(m_xSize, m_ySize);
    glVertex2i(m_xSize - xTextureSize - 40, m_ySize);
    glEnd();

    glEnable(GL_TEXTURE_2D);    
    glColor(0xFFFFFFFF);
    Render_DrawSprite(m_actionInfiltrateTexture, m_xSize - xTextureSize - 10, 10 + (yTextureSize + 8) * 0 );
    Render_DrawSprite(m_actionInfiltrateTexture, m_xSize - xTextureSize - 10, 10 + (yTextureSize + 8) * 1 );
    Render_DrawSprite(m_actionInfiltrateTexture, m_xSize - xTextureSize - 10, 10 + (yTextureSize + 8) * 2 );
    Render_DrawSprite(m_actionInfiltrateTexture, m_xSize - xTextureSize - 10, 10 + (yTextureSize + 8) * 3 );
    */

    glDisable(GL_TEXTURE_2D);
    glColor(0x80000000);
    glBegin(GL_QUADS);
    glVertex2i(0, m_ySize - yStatusBarSize);
    glVertex2i(m_xSize, m_ySize - yStatusBarSize);
    glVertex2i(m_xSize, m_ySize);
    glVertex2i(0, m_ySize);
    glEnd();

    glEnable(GL_TEXTURE_2D);    

    for (int i = 0; i < ButtonId_NumButtons; ++i)
    {
        if (m_button[i].enabled)
        {
            int xButton, yButton, xButtonSize, yButtonSize;
            GetButtonRect((ButtonId)i, xButton, yButton, xButtonSize, yButtonSize);
            glColor(0xFFFFFFFF);
            int buttonOffset = 0;
            int shadowOffset = 10;
            Render_DrawSprite( m_buttonShadowTexture, xButton + shadowOffset, yButton + shadowOffset );
            if (m_activeButton == i && m_activeButtonDown)
            {
                buttonOffset = 5;
            }
            Render_DrawSprite( m_buttonTexture[i], xButton + buttonOffset, yButton + buttonOffset );
        }
    }

}

void ClientGame::OnMouseDown(int x, int y, int button)
{
    if (button == 1)
    {

        ButtonId buttonId = GetButtonAtPoint(x, y);

        if (buttonId != ButtonId_None)
        {
            m_activeButton = buttonId;
            m_activeButtonDown = true;
            m_state = State_Button;
        }
        else if (y < m_ySize - yStatusBarSize)
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
        if (y < m_ySize - yStatusBarSize)
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

}

void ClientGame::OnMouseUp(int x, int y, int button)
{

    if (button == 1)
    {
        if (m_state == State_Button)
        {
            UpdateActiveButton(x, y);
            m_state = State_Idle;

            if (m_activeButtonDown)
            {
                m_activeButtonDown = false;
                OnButtonPressed(m_activeButton);
            }
            m_activeButton = ButtonId_None;
        }
    }

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
    if (m_state == State_Button)
    {
        UpdateActiveButton(x, y);
    }
    if (m_state == State_Panning)
    {
        int xDelta = m_stateX - x;
        int yDelta = m_stateY - y;
        m_mapX += xDelta * m_mapScale;
        m_mapY += yDelta * m_mapScale;
        m_stateX = x;
        m_stateY = y;
    }
    else
    {
        int xWorld, yWorld;
        ScreenToWorld(x, y, xWorld, yWorld);
        m_hoverStop = m_map.GetStopForPoint( Vec2(xWorld, yWorld) );
    }
}

void ClientGame::OnButtonPressed(ButtonId buttonId)
{
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
        case Protocol::PacketType_InitializeGame:
            {
                Protocol::InitializeGamePacket* packet = static_cast<Protocol::InitializeGamePacket*>(data);
                OnInitializeGame(*packet);
            }
            break;

        case Protocol::PacketType_State:
            {
                Protocol::StatePacket* packet = static_cast<Protocol::StatePacket*>(data);
                m_testState.Deserialize(packet->data, packet->header.dataSize);
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

void ClientGame::OnInitializeGame(Protocol::InitializeGamePacket& packet)
{
    LogDebug("Initializing game with seed %i", packet.mapSeed);
    m_map.Generate(xMapSize, yMapSize, packet.mapSeed);
}

void ClientGame::GetButtonRect(ButtonId buttonId, int& x, int& y, int& xSize, int& ySize) const
{
    int numButtons = 0;
    xSize = m_buttonShadowTexture.xSize;
    ySize = m_buttonShadowTexture.ySize;
    for (int i = 0; i < buttonId; ++i)
    {
        if (m_button[i].enabled)
        {
            ++numButtons;
        }
    }
    x = 10 + (xSize + 8) * numButtons;
    y = m_ySize - ySize - 20;
}

void ClientGame::UpdateActiveButtons()
{
    for (int i = 0; i < ButtonId_NumButtons; ++i)
    {
        m_button[i].enabled = true;
    }
    m_activeButton = ButtonId_None;
    m_activeButtonDown = false;
}

ClientGame::ButtonId ClientGame::GetButtonAtPoint(int x, int y) const
{
    for (int i = 0; i < ButtonId_NumButtons; ++i)
    {
        if (m_button[i].enabled)
        {
            int xButton, yButton, xButtonSize, yButtonSize;
            GetButtonRect((ButtonId)i, xButton, yButton, xButtonSize, yButtonSize);

            if (x >= xButton && x <= xButton + xButtonSize &&
                y >= yButton && y <= yButton + yButtonSize)
            {
                return (ButtonId)i;
            }
        }
    }
    return ButtonId_None;
}

void ClientGame::UpdateActiveButton(int x, int y)
{
    m_activeButtonDown = (GetButtonAtPoint(x, y) == m_activeButton);
}