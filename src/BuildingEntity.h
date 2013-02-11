#ifndef GAME_BUILDING_ENTITY_H
#define GAME_BUILDING_ENTITY_H

#include "Entity.h"
#include "Map.h"

// Safe house
class BuildingEntity : public Entity
{

public:

    enum { TypeId = EntityTypeId_Building };
    
    BuildingEntity();

    int             m_stop;
    bool            m_raided;
    int             m_numIntels;

};

#endif
