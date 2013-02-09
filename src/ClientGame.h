#ifndef GAME_CLIENT_GAME_H
#define GAME_CLIENT_GAME_H

#include "Texture.h"

class ClientGame
{

public:

    ClientGame(int xSize, int ySize);

    void LoadResources();
    void Render() const;

    void OnMouseDown(int x, int y, int button);
    void OnMouseUp(int x, int y, int button);
    void OnMouseMove(int x, int y);

private:

    void ScreenToWorld(int xScreen, int yScreen, int& xWorld, int& yWorld) const;

    // Sets the zoom for the map and adjusts the panning so the specified point
    // stays in the same place on the screen.
    void SetMapScale(int scale, int xScreen, int yScreen);

private:

    enum State
    {
        State_Idle,
        State_Panning,
    };

    int         m_xSize;
    int         m_ySize;

    int         m_mapScale;
    int         m_mapX;
    int         m_mapY;

    State       m_state;
    int         m_stateX;
    int         m_stateY;

    Texture     m_mapTexture;
    Texture     m_agentTexture;

    int         m_blipX;
    int         m_blipY;

};

#endif