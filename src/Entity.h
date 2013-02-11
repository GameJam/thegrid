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

    template<class T>
    T* Cast();

    template<class T>
    const T* Cast() const;

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

template<class T>
T* Entity::Cast()
{
    if (m_typeId == T::TypeId)
    {
        return static_cast<T*>(this);
    }
    else
    {
        return NULL;
    }
}

template<class T>
const T* Entity::Cast() const
{
    if (m_typeId == T::TypeId)
    {
        return static_cast<const T*>(this);
    }
    else
    {
        return NULL;
    }
}

#endif
