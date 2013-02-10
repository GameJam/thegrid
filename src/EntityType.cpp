#include "EntityType.h"

#include "Entity.h"

#include "BuildingEntity.h"
#include "PlayerEntity.h"


EntityTypeId EntityType::GetTypeId()
{
    return m_typeId;
}
