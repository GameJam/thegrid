#include "BuildingEntity.h"

BuildingEntity::BuildingEntity() : Entity(EntityTypeId_Building)
{
    m_position = Vec2(0.0f, 0.0f);
}
