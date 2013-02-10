#include "EntityState.h"

#include "Entity.h"

#include <assert.h>

struct SerializeHeader
{
    size_t  numEntities;
    float   time;
};


EntityState::EntityState(EntityTypeRegistry* typeRegistry)
{
    m_nextEntityId = 1;  
    m_typeRegistry = typeRegistry;
}

void EntityState::SetTime(float time)
{
    m_time = time;
}

float EntityState::GetTime() const
{
    return m_time;
}

int EntityState::GetNumEntities() const
{
    return static_cast<int>(m_entities.size());
}

Entity* EntityState::GetEntity(int entityIndex)
{
    return m_entities[entityIndex];
}

const Entity* EntityState::GetEntity(int entityIndex) const
{
    return m_entities[entityIndex];
}

size_t EntityState::GetSerializedSize(int clientId) const
{
    size_t size = sizeof(SerializeHeader);

    for (size_t i = 0; i < m_entities.size(); ++i)
    {
        Entity* entity = m_entities[i];
        if (entity->GetOwnerId() == clientId || entity->GetOwnerId() == -1)
        {
            EntityType* entityType = m_typeRegistry->GetType(entity->GetTypeId());
            size += entityType->GetSerializedSize(entity) + sizeof(EntityTypeId);
        }
    }

    return size;
}

void EntityState::Serialize(int clientId, void* buffer, size_t size) const
{

    SerializeHeader* header = static_cast<SerializeHeader*>(buffer);
    header->time = m_time;

    char* entityBuffer = reinterpret_cast<char*>(header + 1);
    size_t numEntities = 0;
    for (size_t i = 0; i < m_entities.size(); ++i)
    {
        Entity* entity = m_entities[i];

        if (entity->GetOwnerId() == clientId || entity->GetOwnerId() == -1)
        {
            EntityTypeId typeId = entity->GetTypeId();

            *reinterpret_cast<EntityTypeId*>(entityBuffer) = typeId;
            entityBuffer += sizeof(EntityTypeId);

            EntityType* entityType = m_typeRegistry->GetType(typeId);
            entityBuffer += entityType->Serialize(m_entities[i], entityBuffer);

            ++numEntities;
        }
    }

    header->numEntities = numEntities;
    assert(entityBuffer == static_cast<char*>(buffer) + size);

}

void EntityState::Deserialize(const void* buffer, size_t size)
{

    const SerializeHeader* header = static_cast<const SerializeHeader*>(buffer);
    m_time = header->time;

    if (m_entities.size() < header->numEntities)
    {
        m_entities.resize(header->numEntities, NULL);
    }    

    const char* entityBuffer = reinterpret_cast<const char*>(header + 1);
    for (size_t i = 0; i < header->numEntities; ++i)
    {
        EntityTypeId typeId = *reinterpret_cast<const EntityTypeId*>(entityBuffer);
        entityBuffer += sizeof(EntityTypeId);

        EntityType* entityType = m_typeRegistry->GetType(typeId);

        if (m_entities[i] == NULL)
        {
            m_entities[i] = entityType->Create(-1);
        }
        else if (m_entities[i]->GetTypeId() != typeId)
        {
            delete m_entities[i];
            m_entities[i] = entityType->Create(-1);
        }

        entityBuffer += entityType->Deserialize(m_entities[i], entityBuffer);
    }

    if (m_entities.size() > header->numEntities)
    {
        for (size_t i = header->numEntities; i < m_entities.size(); ++i)
        {
            delete m_entities[i];
        }

        m_entities.resize(header->numEntities);
    }

    assert(entityBuffer == static_cast<const char*>(buffer) + size);

}

Entity* EntityState::CreateEntity(EntityTypeId typeId, int ownerId)
{

    EntityType* entityType = m_typeRegistry->GetType(typeId);
    Entity* entity = entityType->Create(m_nextEntityId);
    ++m_nextEntityId;
    entity->SetOwnerId(ownerId);
    m_entities.push_back(entity);
    return entity;

}
