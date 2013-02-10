#include "Entity.h"

void Entity::SetId(int id)
{
    m_id = id;
}

int Entity::GetId() const
{
    return m_id;
}

void Entity::SetTypeId(EntityTypeId typeId)
{
    m_typeId = typeId;
}

EntityTypeId Entity::GetTypeId() const
{
    return m_typeId;
}