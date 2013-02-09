#include "ClientGame.h"

ClientGame::ClientGame(int xSize, int ySize)
{
    m_mapScale  = 1;
    m_mapX      = 0;
    m_mapY      = 0;
    m_xSize     = xSize;
    m_ySize     = ySize;
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
            { &m_mapTexture,   "assets/map_subway.jpg"    },
            { &m_agentTexture, "assets/agent.png"         },
        };

    int numTextures = sizeof(load) / sizeof(TextureLoad);
    for (int i = 0; i < numTextures; ++i)
    {   
        Texture_Load(*load[i].texture, load[i].fileName);
    }

}

void ClientGame::Render() const
{

    Render_Begin(0, 0, m_xSize * m_mapScale, m_ySize * m_mapScale);

    Render_DrawSprite(m_mapTexture, 0, 0);
    Render_DrawSprite(m_agentTexture, 50, 50);

    Render_End();

}

void ClientGame::OnMouseDown(int x, int y, int button)
{
    if (m_mapScale == 1)
    {
        m_mapScale = 2;
    }
    else
    {
        m_mapScale = 1;
    }
}
