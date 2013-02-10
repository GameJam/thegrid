#ifndef GAME_BUILDING_ENTITY_H
#define GAME_BUILDING_ENTITY_H

#include "Entity.h"
#include "Vec2.h"
#include "Map.h"

// Safe house
class BuildingEntity : public Entity
{
public:

    BuildingEntity();
    
    int             m_stop;

};

#endif
