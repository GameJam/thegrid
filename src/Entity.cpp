#include "Entity.h"

Entity::Entity()
{
    m_id = -1;
    m_ownerId = -1;
    m_typeId = EntityTypeId_Invalid;
}

int Entity::GetId() const
{
    return m_id;
}

EntityTypeId Entity::GetTypeId() const
{
    return m_typeId;
}

void Entity::SetOwnerId(int clientId)
{
    m_ownerId = clientId;
}

int Entity::GetOwnerId() const
{
    return m_ownerId;
}
