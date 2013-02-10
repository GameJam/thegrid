#ifndef GAME_ENTITY_TYPE_REGISTRY
#define GAME_ENTITY_TYPE_REGISTRY

#include <vector>

class EntityType;

enum EntityTypeId
{
    EntityTypeId_Agent,
    EntityTypeId_Building,
    EntityTypeId_Player,
    EntityTypeId_Count,
    EntityTypeId_Invalid = -1
};

class EntityTypeRegistry
{
public:
    EntityTypeRegistry();
    ~EntityTypeRegistry();

    void RegisterType(EntityType* entityType);
    EntityType* GetType(EntityTypeId typeId);

private:
    typedef std::vector<EntityType*> EntityTypeList;
    EntityTypeList m_types;
};

#endif
