#ifndef GAME_CLIENT_WORLD_STATE_H
#define GAME_CLIENT_WORLD_STATE_H

#include "EntityType.h"

#include <vector>

class Entity;

class ClientWorldState
{

public:

    ClientWorldState(EntityTypeList* entityTypeList);

    void SetClientId(int clientId);
    int GetClientId() const;

    void SetTime(float time);
    float GetTime() const;

    int GetNumEntities() const;
    Entity* GetEntity(int entityIndex);    
    const Entity* GetEntity(int entityIndex) const;
    
    void AddEntity(Entity* entity);

    size_t GetSerializedSize() const;
    void Serialize(void* buffer, size_t size) const;
    void Deserialize(const void* buffer, size_t size);

private:

    typedef std::vector<Entity*> EntityList;

    int             m_clientId;
    float           m_time;
    EntityList      m_entities;
    EntityTypeList* m_entityTypeList;
    int             m_nextEntityId;

};

#endif
