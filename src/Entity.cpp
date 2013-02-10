#include "Entity.h"

Entity::Entity(EntityTypeId typeId)
{
    m_id = -1;
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

EntityTypeId Entity::GetTypeId() const
{
    return m_typeId;
}