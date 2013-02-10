#ifndef GAME_ENTITY_H
#define GAME_ENTITY_H

#include "EntityType.h"

class Entity
{

public:

    void SetId(int id);
    int GetId() const;

    void SetTypeId(EntityTypeId typeId);
    EntityTypeId GetTypeId() const;

protected:

    int m_id;
    EntityTypeId m_typeId;

};


class TestEntity : public Entity
{

public:
    int clientId;
    int x;
    int y;

};


#endif
