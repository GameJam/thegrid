#ifndef GAME_CLIENT_WORLD_STATE_H
#define GAME_CLIENT_WORLD_STATE_H

#include "EntityType.h"

#include <vector>

class Entity;

class EntityState
{

public:

    EntityState(EntityTypeList* entityTypeList);

    void SetTime(float time);
    float GetTime() const;

    int GetNumEntities() const;
    Entity* GetEntity(int entityIndex);    
    const Entity* GetEntity(int entityIndex) const;
    
    void AddEntity(Entity* entity);

    size_t GetSerializedSize(int clientId) const;
    void Serialize(int clientId, void* buffer, size_t size) const;
    void Deserialize(const void* buffer, size_t size);

private:

    typedef std::vector<Entity*> EntityList;

    float           m_time;
    EntityList      m_entities;
    EntityTypeList* m_entityTypeList;
    int             m_nextEntityId;

};

#endif
