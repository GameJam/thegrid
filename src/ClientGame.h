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

private:

    int         m_xSize;
    int         m_ySize;

    int         m_mapScale;
    int         m_mapX;
    int         m_mapY;

    Texture     m_mapTexture;
    Texture     m_agentTexture;

};

#endif