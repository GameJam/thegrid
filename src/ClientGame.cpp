#include "ClientGame.h"

ClientGame::ClientGame(int xSize, int ySize)
{
    m_mapScale  = 1;
    m_mapX      = 0;
    m_mapY      = 0;
    m_xSize     = xSize;
    m_ySize     = ySize;
    m_state     = State_Idle;
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

    Render_Begin(m_mapX, m_mapY, m_xSize * m_mapScale, m_ySize * m_mapScale);

    Render_DrawSprite(m_mapTexture, 0, 0);
    Render_DrawSprite(m_agentTexture, 50, 50);

    Render_End();

}

void ClientGame::OnMouseDown(int x, int y, int button)
{
    if (button == 1)
    {
        // Toggle zoom on left click.
        if (m_mapScale == 1)
        {
            m_mapScale = 2;
        }
        else
        {
            m_mapScale = 1;
        }
    }

    if (button == 3)
    {
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