#include "Entity.h"

Entity::Entity(EntityTypeId typeId)
{
    m_id = -1;
    m_ownerId = -1;
    m_typeId = typeId;
}

void Entity::SetId(int id)
{
    m_id = id;
}

int Entity::GetId() const
{
    return m_id;
}

void Entity::SetOwnerId(int clientId)
{
    m_ownerId = clientId;
}

int Entity::GetOwnerId() const
{
    return m_ownerId;
}

EntityTypeId Entity::GetTypeId() const
{
    return m_typeId;
}