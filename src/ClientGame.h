#ifndef GAME_CLIENT_GAME_H
#define GAME_CLIENT_GAME_H

#include "Texture.h"
#include "Font.h"
#include "Map.h"

#include "Host.h"
#include "Protocol.h"
#include "EntityState.h"
#include "EntityType.h"
#include "EntityTypeRegistry.h"

#include <bass.h>

class AgentEntity;

class ClientGame : Host::Handler
{

public:

    ClientGame(int xSize, int ySize);
    ~ClientGame();

    void LoadResources();
    void Render() const;

    void OnMouseDown(int x, int y, int button);
    void OnMouseUp(int x, int y, int button);
    void OnMouseMove(int x, int y);

    void Connect(const char* hostName, int port);

    void Update(float deltaTime);

    virtual void OnConnect(int peerId);
    virtual void OnDisconnect(int peerId);
    virtual void OnPacket(int peerId, int channel, void* data, size_t size);

private:
  
    enum ButtonId
    {
        ButtonId_Capture,
        ButtonId_Stakeout,
        ButtonId_Infiltrate,
        ButtonId_Hack,
        ButtonId_Intel,
        ButtonId_NumButtons,
        ButtonId_None,
    };

    enum MapState
    {
        State_Idle,
        State_Panning,
        State_Button,
    };

    enum GameState
    {
        GameState_WaitingForServer,
        GameState_Playing,
        GameState_GameOver
    };

    struct Button
    {
        bool    enabled;  
    };

    void ScreenToWorld(int xScreen, int yScreen, int& xWorld, int& yWorld) const;

    // Sets the zoom for the map and adjusts the panning so the specified point
    // stays in the same place on the screen.
    void SetMapScale(int scale, int xScreen, int yScreen);

    // Moves the map so that the specified world point is in the center of the
    // screen.
    void CenterMap(int xWorld, int yWorld);

    void SendOrder(Protocol::OrderPacket& order);

    void OnInitializeGame(Protocol::InitializeGamePacket& packet);

    void GetButtonRect(ButtonId buttonId, int& x, int& y, int& xSize, int& ySize) const;

    void UpdateActiveButtons();

    ButtonId GetButtonAtPoint(int x, int y) const;

    void OnButtonPressed(ButtonId buttonId);

    void UpdateActiveButton(int x, int y);

    void PlaySample(HSAMPLE sample);

    int GetAgentUnderCursor(int xScreen, int yScreen) const;

    Vec2 GetAgentPosition(const AgentEntity* agent) const;

    StructureType GetStructureAtStop(int stop) const;

    const Entity* GetEntity(int id) const;
    
    void EndGame(bool isWinner);

private:

    static const Protocol::Order kButtonToOrder[ButtonId_NumButtons];

    float               m_time;
    int                 m_clientId;
    int                 m_xSize;
    int                 m_ySize;

    int                 m_mapScale;
    int                 m_mapX;
    int                 m_mapY;

    HSTREAM             m_music;
    HSAMPLE             m_soundAction;
    HSAMPLE             m_soundDeath;
    HSAMPLE             m_soundDrop;
    HSAMPLE             m_soundHack;
    HSAMPLE             m_soundPickup;
    HSAMPLE             m_soundTrain;

    MapState            m_mapState;
    ButtonId            m_activeButton;
    bool                m_activeButtonDown;
    int                 m_stateX;
    int                 m_stateY;

    Texture             m_agentTexture;
    Texture             m_agentStakeoutTexture;
    Texture             m_agentIntelTexture;
    Texture             m_intelTexture;
    Texture             m_buildingTowerTexture;
    Texture             m_buildingBankTexture;
    Texture             m_buildingHouseTexture;
    Texture             m_buildingPoliceTexture;
    Texture             m_buttonTexture[ButtonId_NumButtons];
    Texture             m_buttonShadowTexture;
    Texture             m_playerPortraitTexture;
    Texture             m_playerEliminatedTexture;
    Texture             m_playerBankHackedTexture;
    Texture             m_playerCellHackedTexture;
    Texture             m_playerPoliceHackedTexture;

    Button              m_button[ButtonId_NumButtons];

    Font                m_font;

    int                 m_blipX;
    int                 m_blipY;

    GameState           m_gameState;
    Map                 m_map;
    int                 m_xMapSize;
    int                 m_yMapSize;
    int                 m_gridSpacing;

    Host                m_host;
    int                 m_serverId;

    EntityTypeRegistry  m_typeRegistry;
    EntityState         m_state;

    int                 m_hoverStop;
    int                 m_selectedAgent;

    int                 m_maxPlayersInGame;
    float               m_gameOverTime;
    bool                m_isWinner;

};

#endif