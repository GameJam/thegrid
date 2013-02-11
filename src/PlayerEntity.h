#ifndef GAME_PLAYER_ENTITY_H
#define GAME_PLAYER_ENTITY_H

#include "Entity.h"
#include "Map.h"

class PlayerEntity : public Entity
{

public:

    enum { TypeId = EntityTypeId_Player };

    PlayerEntity();

    char    m_name[32];
    int     m_clientId;
    bool    m_eliminated;
    bool    m_hackingBank;
    bool    m_hackingTower;
    bool    m_hackingPolice;
    float   m_nextIntelPing;
    int     m_lastIntelFound;
    int     m_numSafeHouses;
    int     m_numAgents;
    int     m_numIntels;

};

#endif
