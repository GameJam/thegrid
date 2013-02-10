#include "ClientGame.h"
#include "Log.h"
#include "Entity.h"
#include "AgentEntity.h"
#include "BuildingEntity.h"

#include <math.h>
#include <assert.h>

const int yStatusBarSize    = 140;

static void DrawCircle(const Vec2& point, float radius)
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
      m_state(&m_entityTypes)
{

    InitializeEntityTypes(m_entityTypes);

    m_time      = 0;
    m_hasMap    = false;
    m_mapScale  = 1;
    m_xSize     = xSize;
    m_ySize     = ySize;
    m_mapState     = State_Idle;
    m_blipX     = 0;
    m_blipY     = 0;
    m_serverId  = -1;
    m_hoverStop = -1;
    m_xMapSize  = 0;
    m_yMapSize  = 0;
    m_gridSpacing = 0;
    m_selectedAgent = -1;

    UpdateActiveButtons();

    m_music = BASS_StreamCreateFile(FALSE, "assets/get_a_groove.mp3", 0, 0, BASS_SAMPLE_LOOP);
    BASS_ChannelPlay(m_music, TRUE);

}

ClientGame::~ClientGame()
{
    BASS_StreamFree(m_music);
    BASS_SampleFree(m_soundAction);
    BASS_SampleFree(m_soundDeath);
    BASS_SampleFree(m_soundDrop);
    BASS_SampleFree(m_soundHack);
    BASS_SampleFree(m_soundPickup);
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
            { &m_agentIntelTexture,                     "assets/agent_intel.png"        },
            { &m_agentStakeoutTexture,                  "assets/agent_stakeout.png"     },
            { &m_intelTexture,                          "assets/intel.png"              },
            { &m_buildingTowerTexture,                  "assets/building_tower.png"     },
            { &m_buildingBankTexture,                   "assets/building_bank.png"      },
            { &m_buildingHouseTexture,                  "assets/building_safehouse.png" },
            { &m_buildingPoliceTexture,                 "assets/building_police.png"    },
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

    m_soundAction   = BASS_SampleLoad(false, "assets/sound_action.wav", 0, 0, 3, BASS_SAMPLE_OVER_POS);
    m_soundDeath    = BASS_SampleLoad(false, "assets/sound_death.wav", 0, 0, 3, BASS_SAMPLE_OVER_POS);
    m_soundDrop     = BASS_SampleLoad(false, "assets/sound_drop.wav", 0, 0, 3, BASS_SAMPLE_OVER_POS);
    m_soundHack     = BASS_SampleLoad(false, "assets/sound_hack.wav", 0, 0, 3, BASS_SAMPLE_OVER_POS);
    m_soundPickup   = BASS_SampleLoad(false, "assets/sound_pickup.wav", 0, 0, 3, BASS_SAMPLE_OVER_POS);

}

void ClientGame::Render() const
{

    if (!m_hasMap)
    {
        // TODO: proper state management!
        return;
    }

    const unsigned long lineColor[] = 
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
    glVertex2i(m_xMapSize + outerBorder, -outerBorder);
    glVertex2i(m_xMapSize + outerBorder, m_yMapSize + outerBorder);
    glVertex2i(-outerBorder, m_yMapSize + outerBorder);
    glEnd();

    // Outer thick border of the map
    glColor(0xFF7FD6F2);
    glLineWidth(2);
    glBegin(GL_LINE_LOOP);
    glVertex2i(-outerBorder, -outerBorder);
    glVertex2i(m_xMapSize + outerBorder, -outerBorder);
    glVertex2i(m_xMapSize + outerBorder, m_yMapSize + outerBorder);
    glVertex2i(-outerBorder, m_yMapSize + outerBorder);
    glEnd();

    // Grid
    glColor(0xFF7FD6F2);
    glLineWidth(1);
    glBegin(GL_LINES);
    for (int x = 0; x < m_xMapSize / m_gridSpacing; ++x)
    {
        glVertex2i(x * m_gridSpacing, 0);
        glVertex2i(x * m_gridSpacing, m_yMapSize);
    }
    glVertex2i(m_xMapSize, 0);
    glVertex2i(m_xMapSize, m_yMapSize);
    for (int y = 0; y < m_yMapSize / m_gridSpacing; ++y)
    {
        glVertex2i(0, y * m_gridSpacing);
        glVertex2i(m_xMapSize, y * m_gridSpacing);
    }
    glVertex2i(0, m_yMapSize);
    glVertex2i(m_xMapSize, m_yMapSize);
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
            DrawCircle(stop.point, 8.0f + inflate);
            glColor( 0xFFFFFFFF );
            DrawCircle(stop.point, 6.0f + inflate);
        }
        else
        {
            glColor( lineColor[stop.line % numLines] );
            DrawCircle(stop.point, 8.0f + inflate);
        }
    }

    // Draw the legend of the grid.
    Font_BeginDrawing(m_font);
    glColor(0xFF7FD6F2);
    int fontHeight = Font_GetTextHeight(m_font);
    for (int x = 0; x < m_xMapSize / m_gridSpacing; ++x)
    {
        char buffer[32];
        sprintf(buffer, "%d", x);
        int textWidth = Font_GetTextWidth(m_font, buffer);
        Font_DrawText(buffer, x * m_gridSpacing + m_gridSpacing / 2 - textWidth / 2, -outerBorder + 5);
        Font_DrawText(buffer, x * m_gridSpacing + m_gridSpacing / 2 - textWidth / 2, m_yMapSize + 5);
    }
    for (int y = 0; y < m_yMapSize / m_gridSpacing; ++y)
    {
        char buffer[32];
        sprintf(buffer, "%c", 'A' + y);
        Font_DrawText(buffer, -outerBorder + 10, y * m_gridSpacing + m_gridSpacing / 2 - fontHeight / 2);
        Font_DrawText(buffer, m_xMapSize + 10, y * m_gridSpacing + m_gridSpacing / 2 - fontHeight / 2);
    }
    Font_EndDrawing();


    glEnable(GL_TEXTURE_2D);    
    glColor(0xFFFFFFFF);

    // Entities
    for (int i = 0; i < m_state.GetNumEntities(); ++i)
    {
        const Entity* entity = m_state.GetEntity(i);
        
        switch (entity->GetTypeId())
        {
        case EntityTypeId_Agent:
            {
                const AgentEntity* agent = static_cast<const AgentEntity*>(entity);
                Vec2 position = GetAgentPosition(agent);
                if (agent->m_hasIntel)
                {
                    Render_DrawSprite(m_agentIntelTexture, static_cast<int>(position.x) - m_agentIntelTexture.xSize / 2, static_cast<int>(position.y) - m_agentIntelTexture.ySize / 2);
                }
                else
                {
                    Render_DrawSprite(m_agentTexture, static_cast<int>(position.x) - m_agentTexture.xSize / 2, static_cast<int>(position.y) - m_agentTexture.ySize / 2);
                }
            }
            break;
        case EntityTypeId_Building:
            {
                const BuildingEntity* building = static_cast<const BuildingEntity*>(entity);
                Vec2 position = m_map.GetStop(building->m_stop).point;
                
                const Texture* texture = NULL;

                switch (building->m_type)
                {
                case StructureType_Bank:
                    texture = &m_buildingBankTexture;
                    break;
                case StructureType_Tower:
                    texture = &m_buildingTowerTexture;
                    break;
                case StructureType_Police:
                    texture = &m_buildingPoliceTexture;
                    break;
                }
                if (texture != NULL)
                {
                    Render_DrawSprite(*texture, static_cast<int>(position.x) - texture->xSize / 2, static_cast<int>(position.y) - texture->ySize / 2);
                }
            }
            break;
        }
        
    }

    // Draw the UI.

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, m_xSize, m_ySize, 0);

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
            m_mapState = State_Button;
        }
        else if (y < m_ySize - yStatusBarSize)
        {
            bool hasSelection = (m_selectedAgent != -1);
            int agentUnderCursor = GetAgentUnderCursor(x, y);
            
            int xWorld, yWorld;
            ScreenToWorld(x, y, xWorld, yWorld);
            int stopUnderCursor = m_map.GetStopForPoint( Vec2(static_cast<float>(xWorld), static_cast<float>(yWorld)) );

            if (agentUnderCursor == -1 && stopUnderCursor != -1)
            {
                // Order to move to station
            }
            else
            {
                m_selectedAgent = agentUnderCursor;
            }

            UpdateActiveButtons();

            // First click deselects.
            if (m_selectedAgent == -1 && !hasSelection)
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
            if (m_mapState == State_Idle)
            {
                m_mapState = State_Panning;
                m_stateX = x;
                m_stateY = y;
            }
        }
    }

}

int ClientGame::GetAgentUnderCursor(int xScreen, int yScreen) const
{

    int xWorld, yWorld;
    ScreenToWorld(xScreen, yScreen, xWorld, yWorld);

    for (int i = 0; i < m_state.GetNumEntities(); ++i)
    {
        const Entity* entity = m_state.GetEntity(i);
        if (entity->GetTypeId() == EntityTypeId_Agent)
        {
            const AgentEntity* agent = static_cast<const AgentEntity*>(entity);
            Vec2 position = GetAgentPosition(agent);

            if (xWorld >= position.x - m_agentTexture.xSize / 2 &&
                yWorld >= position.y - m_agentTexture.ySize / 2 &&
                xWorld <= position.x + m_agentTexture.xSize / 2 &&
                yWorld <= position.y + m_agentTexture.ySize / 2)
            {
                return agent->GetId();
            }
        }
        
    }

    return -1;

}

Vec2 ClientGame::GetAgentPosition(const AgentEntity* agent) const
{
    int stop = agent->m_currentStop;
    if (stop != -1)
    {
        return m_map.GetStop(stop).point;
    }
    return Vec2(0.0f, 0.0f);
}

void ClientGame::OnMouseUp(int x, int y, int button)
{

    if (button == 1)
    {
        if (m_mapState == State_Button)
        {
            UpdateActiveButton(x, y);
            m_mapState = State_Idle;

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
        if (m_mapState == State_Panning)
        {
            m_mapState = State_Idle;
        }
    }
}

void ClientGame::OnMouseMove(int x, int y)
{
    if (m_mapState == State_Button)
    {
        UpdateActiveButton(x, y);
    }
    if (m_mapState == State_Panning)
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
        if (GetAgentUnderCursor(x, y) == -1)
        {
            m_hoverStop = m_map.GetStopForPoint( Vec2(static_cast<float>(xWorld), static_cast<float>(yWorld)) );
        }
        else
        {
            m_hoverStop = -1;
        }
    }
}

void ClientGame::OnButtonPressed(ButtonId buttonId)
{
    switch (buttonId)
    {
    case ButtonId_Infiltrate:
        PlaySample(m_soundAction);
        break;
    case ButtonId_Capture:
        PlaySample(m_soundDeath);
        break;
    case ButtonId_Stakeout:
        PlaySample(m_soundAction);
        break;
    case ButtonId_Hack:
        PlaySample(m_soundHack);
        break;
    case ButtonId_Intel:
        PlaySample(m_soundPickup);
        break;
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

void ClientGame::Update(float deltaTime)
{
    m_time += deltaTime;
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
                m_state.Deserialize(packet->data, packet->header.dataSize);
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
    m_xMapSize = packet.xMapSize;
    m_yMapSize = packet.yMapSize;
    m_gridSpacing = packet.gridSpacing;
    m_map.Generate(m_xMapSize, m_yMapSize, packet.mapSeed);
    CenterMap(m_xMapSize / 2, m_yMapSize / 2);

    m_hasMap = true;
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
    bool selection = (m_selectedAgent != -1);
    for (int i = 0; i < ButtonId_NumButtons; ++i)
    {
        m_button[i].enabled = selection;
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

void ClientGame::PlaySample(HSAMPLE sample)
{
    HCHANNEL channel = BASS_SampleGetChannel(sample, false);
    BASS_ChannelPlay(channel, true);
}
