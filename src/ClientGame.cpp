#include "ClientGame.h"
#include "Log.h"
#include "Entity.h"
#include "AgentEntity.h"
#include "BuildingEntity.h"
#include "PlayerEntity.h"
#include "Utility.h"

#include <math.h>
#include <assert.h>

const int yStatusBarSize    = 140;

const Protocol::Order ClientGame::kButtonToOrder[ButtonId_NumButtons] = 
{
    Protocol::Order_Capture,
    Protocol::Order_Stakeout,
    Protocol::Order_Infiltrate,
    Protocol::Order_Hack,
    Protocol::Order_Intel
};

static float EaseInOutQuad(float t, float b, float c, float d) 
{
    t /= d/2;
    if (t < 1) return c/2*t*t + b;
    t--;
    return -c/2 * (t*(t-2) - 1) + b;
};

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
      m_state(&m_typeRegistry)
{

    m_time      = 0;
    m_clientId  = -1;
    m_gameState = GameState_WaitingForServer;
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
    m_maxPlayersInGame = 0;
    m_gameOverTime = 0;
    m_isWinner = false;

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
    BASS_SampleFree(m_soundTrain);
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
            { &m_playerPortraitTexture,                 "assets/player_portrait.png"    },
            { &m_playerEliminatedTexture,               "assets/player_eliminated.png"  },
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
    m_soundTrain    = BASS_SampleLoad(false, "assets/sound_train.wav", 0, 0, 3, BASS_SAMPLE_OVER_POS);

}

void ClientGame::Render() const
{

    if (m_gameState == GameState_WaitingForServer)
    {
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

    // Buildings.
    glEnable(GL_TEXTURE_2D);
    glColor(0xFFFFFFFF);
    for (int i = 0; i < m_map.GetNumStops(); ++i)
    {
        const Stop& stop = m_map.GetStop(i);
        Vec2 position = stop.point;
                
        const Texture* texture = NULL;
        switch (stop.structureType)
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
    for (int i = 0; i < m_state.GetNumEntities(); ++i)
    {
        const Entity* entity = m_state.GetEntity(i);
        switch (entity->GetTypeId())
        {
        case EntityTypeId_Building:
            {
                const BuildingEntity* building = static_cast<const BuildingEntity*>(entity);
                Vec2 position = m_map.GetStop(building->m_stop).point;
                Render_DrawSprite(m_buildingHouseTexture, static_cast<int>(position.x) - m_buildingHouseTexture.xSize / 2, static_cast<int>(position.y) - m_buildingHouseTexture.ySize / 2);
            }
            break;
        }
    }
    glDisable(GL_TEXTURE_2D);

    // Rails.
    glLineWidth(8.0f / m_mapScale);
    glBegin(GL_LINES);
    for (int i = 0; i < m_map.GetNumRails(); ++i)
    {
        const Rail& rail  = m_map.GetRail(i);
        const Stop& stop1 = m_map.GetStop(rail.stop1);
        const Stop& stop2 = m_map.GetStop(rail.stop2);
        assert(rail.line >= 0);
        unsigned long color = lineColor[rail.line % numLines];
        // Draw the rails partially transparent to make the buldings more readable.
        color = (color & 0x00FFFFFF) | 0x90000000;
        glColor( color );
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

    const int maxPlayers = 32;
    const PlayerEntity* player[maxPlayers] = { NULL };
    int numPlayers = m_state.GetEntitiesWithType(player, maxPlayers);

    // Show the players.

    glDisable(GL_TEXTURE_2D);
    glColor(0x80000000);
    glBegin(GL_QUADS);
    for (int i = 0; i < numPlayers; ++i)
    {
        int x = m_xSize - 300;
        int y = 10 + 130 * i;
        glVertex2i( x, y );   
        glVertex2i( m_xSize, y );   
        glVertex2i( m_xSize, y + 115 );   
        glVertex2i( x, y + 115 );   
    }
    glEnd();

    glEnable(GL_TEXTURE_2D);
    glColor(0xFFFFFFFF);
    for (int i = 0; i < numPlayers; ++i)
    {
        Render_DrawSprite(m_playerPortraitTexture,
            m_xSize - 290,
            20 + 130 * i);
    }

    Font_BeginDrawing(m_font);
    glColor(0xFF000000);

    for (int i = 0; i < numPlayers; ++i)
    {
        Font_DrawText(player[i]->m_name,
            m_xSize - m_playerPortraitTexture.xSize - 60,
            20 + 130 * i);
    }

    glColor(0xFF000000);
    
    if (m_gameState == GameState_GameOver)
    {
        Font_DrawText(m_isWinner ? "You WIN!" : "GAME OVER", 10, 10);
    }

    Font_EndDrawing();

    glEnable(GL_TEXTURE_2D);
    glColor(0xFFFFFFFF);
    for (int i = 0; i < numPlayers; ++i)
    {
        if (player[i]->m_eliminated)
        {
            Render_DrawSprite(m_playerEliminatedTexture,
                m_xSize - 250,
                20 + 130 * i);
        }
    }

}

void ClientGame::OnMouseDown(int x, int y, int button)
{
    
    if (m_gameState != GameState_Playing)
    {
        return;
    }

    if (button == 1)
    {

        ButtonId buttonId = GetButtonAtPoint(x, y);

        if (buttonId != ButtonId_None)
        {
            m_activeButton = buttonId;
            m_activeButtonDown = true;
            m_mapState = State_Button;

            Protocol::OrderPacket order;
            order.order = kButtonToOrder[buttonId];
            order.agentId = m_selectedAgent;
            SendOrder(order);
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
                Protocol::OrderPacket order;
                order.order = Protocol::Order_MoveTo;
                order.agentId = m_selectedAgent;
                order.targetStop = stopUnderCursor;
                SendOrder(order);
                // TODO: play in response from the server?
                PlaySample(m_soundTrain);
            }
            else
            {
                m_selectedAgent = agentUnderCursor;
            }

            UpdateActiveButtons();
        }

    }

    // Zoom.
    if (button == 4)
    {
        SetMapScale(1, x, y);
    }
    if (button == 5)
    {
        SetMapScale(2, x, y);
    }


    if (button == 2)
    {
        int blipX, blipY;
        ScreenToWorld(x, y, blipX, blipY);

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
    
    int index = 0;
    const AgentEntity* agent = NULL;
    while (m_state.GetNextEntityWithType(index, agent))
    {
        Vec2 position = GetAgentPosition(agent);

        if (agent->GetId() != m_selectedAgent &&
            xWorld >= position.x - m_agentTexture.xSize / 2 &&
            yWorld >= position.y - m_agentTexture.ySize / 2 &&
            xWorld <= position.x + m_agentTexture.xSize / 2 &&
            yWorld <= position.y + m_agentTexture.ySize / 2)
        {
            return agent->GetId();
        }        
    }

    return -1;

}

Vec2 ClientGame::GetAgentPosition(const AgentEntity* agent) const
{
    int stop = agent->m_currentStop;
    int targetStop = agent->m_targetStop;

    if (targetStop != -1)
    {
        Vec2 from = m_map.GetStop(stop).point;
        Vec2 to = m_map.GetStop(targetStop).point;

        float t = (m_time - agent->m_departureTime) / (agent->m_arrivalTime - agent->m_departureTime);
        t = Clamp(t, 0.0f, 1.0f);
        t = EaseInOutQuad(t, 0.0f, 1.0f, 1.0f);
        
        return Lerp(from, to, t);
    }
    if (stop != -1)
    {
        return m_map.GetStop(stop).point;
    }
    return Vec2(0.0f, 0.0f);
}

void ClientGame::OnMouseUp(int x, int y, int button)
{

    if (m_gameState != GameState_Playing)
    {
        return;
    }

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

    if (m_gameState != GameState_Playing)
    {
        return;
    }

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

    UpdateActiveButtons();
    
    switch (m_gameState)
    {
    case GameState_Playing:
        // Check for end game
        int numActivePlayers = 0;
        int index = 0;
        const PlayerEntity* player;
        while (m_state.GetNextEntityWithType(index, player))
        {
            if (!player->m_eliminated)
            {
                ++numActivePlayers;
                continue;
            }
            else if (player->m_clientId == m_clientId)
            {
                // We're out!
                EndGame(false);
                break;
            }
        }
        
        m_maxPlayersInGame = Max(m_maxPlayersInGame, numActivePlayers);
        if (m_gameState != GameState_GameOver && numActivePlayers == 1 && m_maxPlayersInGame > 1)
        {
            // Last one standing
            EndGame(true);
        }
        break;
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
    m_time = packet.time;
    m_clientId = packet.clientId;
    m_xMapSize = packet.xMapSize;
    m_yMapSize = packet.yMapSize;
    m_gridSpacing = packet.gridSpacing;
    m_map.Generate(m_xMapSize, m_yMapSize, packet.mapSeed);
    CenterMap(m_xMapSize / 2, m_yMapSize / 2);

    m_gameState = GameState_Playing;
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

    StructureType structure = StructureType_None;

    if (selection)
    {
        const Entity* entity = GetEntity(m_selectedAgent);
        if (entity == NULL)
        {
            selection = false;
        }
        else
        {
            assert(entity->GetTypeId() == EntityTypeId_Agent);
            const AgentEntity* agent = static_cast<const AgentEntity*>(entity);
            if (agent->m_currentStop != -1)
            {
                structure = GetStructureAtStop(agent->m_currentStop);
            }
        }
    }

    bool buttonEnabled[ButtonId_NumButtons];
    for (int i = 0; i < ButtonId_NumButtons; ++i)
    {
        buttonEnabled[i] = selection;
    }

    if (structure == StructureType_House)
    {
        buttonEnabled[ButtonId_Hack] = false;
    }
    else
    {
        buttonEnabled[ButtonId_Intel] = false;
    }

    if (structure != StructureType_None)
    {
        buttonEnabled[ButtonId_Infiltrate] = false;
    }
    else
    {
        buttonEnabled[ButtonId_Hack] = false;
    }

    for (int i = 0; i < ButtonId_NumButtons; ++i)
    {
        m_button[i].enabled = buttonEnabled[i];
        // If the active button is going away, remove its current state.
        if (m_activeButton == i)
        {
            if (!m_button[i].enabled)
            {
                m_activeButton = ButtonId_None;
                m_activeButtonDown = false;
            }
        }
    }
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

StructureType ClientGame::GetStructureAtStop(int stop) const
{
    StructureType structureType = m_map.GetStop(stop).structureType;   

    if (structureType != StructureType_None)
    {
        return structureType;
    }

    int index = 0;
    const BuildingEntity* entity;
    while (m_state.GetNextEntityWithType(index, entity))
    {
        if (entity->m_stop == stop)
        {
            return StructureType_House;
        }
    }
    return StructureType_None;
}

const Entity* ClientGame::GetEntity(int id) const
{
    for (int i = 0; i < m_state.GetNumEntities(); ++i)
    {
        const Entity* entity = m_state.GetEntity(i);
        if (entity->GetId() == id)
        {
            return entity;
        }
    }
    return NULL;
}

void ClientGame::EndGame(bool isWinner)
{
    m_gameState = GameState_GameOver;
    m_gameOverTime = m_time;
    m_isWinner = isWinner;
}