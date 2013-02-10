#ifndef GAME_BUILDING_ENTITY_H
#define GAME_BUILDING_ENTITY_H

#include "Entity.h"
#include "Vec2.h"

class BuildingEntity : public Entity
{
public:

    BuildingEntity();
    
    Vec2    m_position;

};

#endif
