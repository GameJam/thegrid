#ifndef GAME_CLIENT_WORLD_STATE_H
#define GAME_CLIENT_WORLD_STATE_H

#include "EntityType.h"

#include <vector>

class Entity;

class ClientWorldState
{

public:

    ClientWorldState(EntityTypeList* entityTypeList);

    int GetNumEntities() const;
    Entity* GetEntity(int entityIndex);    
    const Entity* GetEntity(int entityIndex) const;
    
    void AddEntity(Entity* entity);

    size_t GetSerializedSize();
    void Serialize(void* buffer, size_t size);
    void Deserialize(const void* buffer, size_t size);

private:

    typedef std::vector<Entity*> EntityList;

    int             m_clientId;
    EntityList      m_entities;
    EntityTypeList* m_entityTypeList;
    int             m_nextEntityId;

};

#endif
