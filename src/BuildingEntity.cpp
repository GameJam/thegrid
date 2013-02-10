#include "BuildingEntity.h"

BuildingEntity::BuildingEntity() : Entity(EntityTypeId_Building)
{
    m_stop = 0;
    m_type = StructureType_None;
}
