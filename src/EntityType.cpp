#include "EntityType.h"

#include "Entity.h"

#include "AgentEntity.h"
#include "BuildingEntity.h"
#include "PlayerEntity.h"

EntityTypeId EntityType::GetTypeId()
{
    return m_typeId;
}

void InitializeEntityTypes(EntityTypeList& entityTypes)
{
    entityTypes.resize(EntityTypeId_Count);
    entityTypes[EntityTypeId_Test] = new StructEntityType<TestEntity>(EntityTypeId_Test, "Test");
    entityTypes[EntityTypeId_Agent] = new StructEntityType<AgentEntity>(EntityTypeId_Agent, "Agent");
    entityTypes[EntityTypeId_Building] = new StructEntityType<BuildingEntity>(EntityTypeId_Building, "Building");
    entityTypes[EntityTypeId_Player] = new StructEntityType<PlayerEntity>(EntityTypeId_Player, "Player");
}
