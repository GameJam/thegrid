#include "EntityTypeRegistry.h"

#include "AgentEntity.h"
#include "BuildingEntity.h"
#include "PlayerEntity.h"

EntityTypeRegistry::EntityTypeRegistry()
{

    m_types.resize(EntityTypeId_Count, NULL);

    RegisterType(new StructEntityType<AgentEntity>);
    RegisterType(new StructEntityType<BuildingEntity>);
    RegisterType(new StructEntityType<PlayerEntity>);

}

EntityTypeRegistry::~EntityTypeRegistry()
{

    for (size_t i = 0; i < m_types.size(); ++i)
    {
        delete m_types[i];
    }
    m_types.clear();

}

void EntityTypeRegistry::RegisterType(EntityType* entityType)
{
    m_types[entityType->GetTypeId()] = entityType;
}

EntityType* EntityTypeRegistry::GetType(EntityTypeId typeId)
{
    return m_types[typeId];
}
