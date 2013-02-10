#ifndef GAME_ENTITY_H
#define GAME_ENTITY_H

#include "EntityType.h"

class Entity
{

public:

    Entity(EntityTypeId typeId);

    void SetId(int id);
    int GetId() const;

    EntityTypeId GetTypeId() const;

protected:

    int m_id;
    EntityTypeId m_typeId;

};


class TestEntity : public Entity
{

public:
    TestEntity() : Entity(EntityTypeId_Test) {}

    int clientId;
    int x;
    int y;

};


#endif
