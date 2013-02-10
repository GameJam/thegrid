#ifndef GAME_CLIENT_GAME_H
#define GAME_CLIENT_GAME_H

#include "Texture.h"
#include "Font.h"
#include "Map.h"

#include "Host.h"
#include "Protocol.h"
#include "ClientWorldState.h"

#include <bass.h>

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

    void Update();

    virtual void OnConnect(int peerId);
    virtual void OnDisconnect(int peerId);
    virtual void OnPacket(int peerId, int channel, void* data, size_t size);

private:
  
    enum ButtonId
    {
        ButtonId_Infiltrate,
        ButtonId_Capture,
        ButtonId_Stakeout,
        ButtonId_Hack,
        ButtonId_Intel,
        ButtonId_NumButtons,
        ButtonId_None,
    };

    enum State
    {
        State_Idle,
        State_Panning,
        State_Button,
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

private:
  
    int         m_xSize;
    int         m_ySize;

    int         m_mapScale;
    int         m_mapX;
    int         m_mapY;

    HSTREAM     m_music;

    State       m_state;
    ButtonId    m_activeButton;
    bool        m_activeButtonDown;
    int         m_stateX;
    int         m_stateY;

    Texture     m_agentTexture;
    Texture     m_buttonTexture[ButtonId_NumButtons];
    Texture     m_buttonShadowTexture;

    Button      m_button[ButtonId_NumButtons];

    Font        m_font;

    int         m_blipX;
    int         m_blipY;

    Map         m_map;

    Host        m_host;
    int         m_serverId;
    ClientWorldState    m_testState;

    int         m_hoverStop;

};

#endif