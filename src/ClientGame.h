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

};

#endif