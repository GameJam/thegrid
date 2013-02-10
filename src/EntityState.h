#ifndef GAME_CLIENT_WORLD_STATE_H
#define GAME_CLIENT_WORLD_STATE_H

#include "EntityType.h"

#include <vector>

class Entity;
class EntityTypeRegistry;

class EntityState
{

public:

    EntityState(EntityTypeRegistry* typeRegistry);

    void SetTime(float time);
    float GetTime() const;

    int GetNumEntities() const;
    Entity* GetEntity(int entityIndex);    
    const Entity* GetEntity(int entityIndex) const;
    
    size_t GetSerializedSize(int clientId) const;
    void Serialize(int clientId, void* buffer, size_t size) const;
    void Deserialize(const void* buffer, size_t size);

    Entity* CreateEntity(EntityTypeId typeId, int ownerId=-1);

    template<class T>
    T* CreateEntity(int ownerId=-1);

    template<class T>
    int GetEntitiesWithType(T* results[], int maxEntitites);

    template<class T>
    int GetEntitiesWithType(const T* results[], int maxEntitites) const;

    template<class T>
    bool GetNextEntityWithType(int& index, const T*& entity) const;

    template<class T>
    bool GetNextEntityWithType(int& index, T*& entity) const;

private:

    typedef std::vector<Entity*> EntityList;

    float               m_time;
    EntityList          m_entities;
    EntityTypeRegistry* m_typeRegistry;
    int                 m_nextEntityId;

};


template<class T>
T* EntityState::CreateEntity(int ownerId)
{
    return static_cast<T*>(CreateEntity(static_cast<EntityTypeId>(T::TypeId), ownerId));
}

template<class T>
int EntityState::GetEntitiesWithType(T* results[], int maxEntities)
{
        
    int numEntities = 0;
    for (size_t i = 0; i < m_entities.size(); ++i)
    {
        if (m_entities[i]->GetTypeId() == T::TypeId)
        {
            results[numEntities] = static_cast<T*>(m_entities[i]);
            ++numEntities;
        }
    }
    return numEntities;

}

template<class T>
int EntityState::GetEntitiesWithType(const T* results[], int maxEntities) const
{

    int numEntities = 0;
    for (size_t i = 0; i < m_entities.size(); ++i)
    {
        if (m_entities[i]->GetTypeId() == T::TypeId)
        {
            results[numEntities] = static_cast<const T*>(m_entities[i]);
            ++numEntities;
        }
    }
    return numEntities;

}

template<class T>
bool EntityState::GetNextEntityWithType(int& index, const T*& typedEntity) const
{

    while (index < static_cast<int>(m_entities.size()))
    {
        Entity* entity = m_entities[index];
        ++index;

        if (entity->GetTypeId() == T::TypeId)
        {
            typedEntity = static_cast<const T*>(entity);
            return true;
        }
    }

    return false;
}

template<class T>
bool EntityState::GetNextEntityWithType(int& index, T*& typedEntity) const
{

    while (index < static_cast<int>(m_entities.size()))
    {
        Entity* entity = m_entities[index];
        ++index;

        if (entity->GetTypeId() == T::TypeId)
        {
            typedEntity = static_cast<T*>(entity);
            return true;
        }
    }

    return false;
}

#endif
