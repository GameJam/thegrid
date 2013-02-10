#ifndef GAME_PLAYER_ENTITY_H
#define GAME_PLAYER_ENTITY_H

#include "Entity.h"
#include "Map.h"

class PlayerEntity : public Entity
{
public:

    PlayerEntity();

    char    m_name[32];
    bool    m_hackingBank;
    bool    m_hackingTower;
    bool    m_hackingPolice;

};

#endif
