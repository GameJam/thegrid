#include "ClientWorldState.h"

#include "Entity.h"

#include <assert.h>

struct SerializeHeader
{
    size_t  numEntities;
    int     clientId;
};


ClientWorldState::ClientWorldState(EntityTypeList* entityTypeList)
{
    m_clientId = -1;
    m_nextEntityId = 1;  
    m_entityTypeList = entityTypeList;
}

void ClientWorldState::SetClientId(int clientId)
{
    m_clientId = clientId;
}

int ClientWorldState::GetClientId() const
{
    return m_clientId;
}

int ClientWorldState::GetNumEntities() const
{
    return static_cast<int>(m_entities.size());
}

Entity* ClientWorldState::GetEntity(int entityIndex)
{
    return m_entities[entityIndex];
}

void ClientWorldState::AddEntity(Entity* entity)
{
    entity->SetId(m_nextEntityId);
    ++m_nextEntityId;
    m_entities.push_back(entity);
}

const Entity* ClientWorldState::GetEntity(int entityIndex) const
{
    return m_entities[entityIndex];
}

size_t ClientWorldState::GetSerializedSize() const
{
    size_t size = sizeof(SerializeHeader);

    for (size_t i = 0; i < m_entities.size(); ++i)
    {
        EntityType* entityType = (*m_entityTypeList)[m_entities[i]->GetTypeId()];
        size += entityType->GetSerializedSize(m_entities[i]) + sizeof(EntityTypeId);
    }

    return size;
}

void ClientWorldState::Serialize(void* buffer, size_t size) const
{

    SerializeHeader* header = static_cast<SerializeHeader*>(buffer);
    header->numEntities = m_entities.size();
    header->clientId = m_clientId;

    char* entityBuffer = reinterpret_cast<char*>(header + 1);
    for (size_t i = 0; i < m_entities.size(); ++i)
    {
        EntityTypeId typeId = m_entities[i]->GetTypeId();

        *reinterpret_cast<EntityTypeId*>(entityBuffer) = typeId;
        entityBuffer += sizeof(EntityTypeId);

        EntityType* entityType = (*m_entityTypeList)[typeId];
        entityBuffer += entityType->Serialize(m_entities[i], entityBuffer);
    }

    assert(entityBuffer == static_cast<char*>(buffer) + size);

}

void ClientWorldState::Deserialize(const void* buffer, size_t size)
{

    const SerializeHeader* header = static_cast<const SerializeHeader*>(buffer);
    m_clientId = header->clientId;

    if (m_entities.size() < header->numEntities)
    {
        m_entities.resize(header->numEntities, NULL);
    }    

    const char* entityBuffer = reinterpret_cast<const char*>(header + 1);
    for (size_t i = 0; i < header->numEntities; ++i)
    {
        EntityTypeId typeId = *reinterpret_cast<const EntityTypeId*>(entityBuffer);
        entityBuffer += sizeof(EntityTypeId);

        EntityType* entityType = (*m_entityTypeList)[typeId];

        if (m_entities[i] == NULL)
        {
            m_entities[i] = entityType->Create();
        }
        else if (m_entities[i]->GetTypeId() != typeId)
        {
            delete m_entities[i];
            m_entities[i] = entityType->Create();
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
