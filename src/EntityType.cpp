#include "EntityType.h"

#include "Entity.h"

#include "AgentEntity.h"

EntityTypeId EntityType::GetTypeId()
{
    return m_typeId;
}

void InitializeEntityTypes(EntityTypeList& entityTypes)
{
    entityTypes.resize(EntityTypeId_Count);
    entityTypes[EntityTypeId_Test] = new StructEntityType<TestEntity>(EntityTypeId_Test, "Test");
    entityTypes[EntityTypeId_Agent] = new StructEntityType<AgentEntity>(EntityTypeId_Agent, "Agent");
}
