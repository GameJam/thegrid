#include "EntityType.h"

#include "Entity.h"

EntityTypeId EntityType::GetTypeId()
{
    return m_typeId;
}

void InitializeEntityTypes(EntityTypeList& entityTypes)
{
    entityTypes.resize(EntityTypeId_Count);
    entityTypes[EntityTypeId_Test] = new StructEntityType<TestEntity>(EntityTypeId_Test, "Test Entity");
}
