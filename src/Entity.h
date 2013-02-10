#ifndef GAME_ENTITY_H
#define GAME_ENTITY_H

#include "EntityType.h"
#include "EntityTypeRegistry.h"

class Entity
{

public:

    template<class T>
    static T* CreateEntity(int entityId, EntityTypeId typeId);

    int GetId() const;
    EntityTypeId GetTypeId() const;

    void SetOwnerId(int clientId);
    int GetOwnerId() const;

protected:

    Entity();

    int             m_id;
    int             m_ownerId;
    EntityTypeId    m_typeId;

};

template<class T>
T* Entity::CreateEntity(int entityId, EntityTypeId typeId)
{
    T* entity = new T();
    entity->m_id = entityId;
    entity->m_typeId = typeId;
    return entity;
}

#endif
