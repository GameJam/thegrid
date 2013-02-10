#ifndef GAME_ENTITY_TYPE_H
#define GAME_ENTITY_TYPE_H

#include <vector>

enum EntityTypeId
{
    EntityTypeId_Test,
    EntityTypeId_Count
};

class Entity;

class EntityType
{

public:

    EntityTypeId GetTypeId();

    virtual Entity* Create()=0;
    virtual size_t GetSerializedSize(Entity* entity) const=0;
    virtual size_t Serialize(const Entity* entity, void *buffer) const=0;
    virtual size_t Deserialize(Entity* entity, const void* buffer)=0;

protected:

    EntityTypeId    m_typeId;
    const char*     m_typeName;

};

template <class T>
class StructEntityType : public EntityType
{

public:
    
    StructEntityType(EntityTypeId typeId, const char* typeName);

    virtual Entity* Create();
    virtual size_t GetSerializedSize(Entity* entity) const;
    virtual size_t Serialize(const Entity* entity, void *buffer) const;
    virtual size_t Deserialize(Entity* entity, const void* buffer);


};

template<class T>
StructEntityType<T>::StructEntityType(EntityTypeId typeId, const char* typeName)
{
    m_typeId = typeId;
    m_typeName = typeName;
}

template<class T>
Entity* StructEntityType<T>::Create()
{
    T* entity = new T();
    entity->SetTypeId(m_typeId);
    return entity;
}

template<class T>
size_t StructEntityType<T>::GetSerializedSize(Entity* entity) const
{
    return sizeof(T);
}

template<class T>
size_t StructEntityType<T>::Serialize(const Entity* entity, void *buffer) const
{
    memcpy(buffer, entity, sizeof(T));
    return sizeof(T);
}

template<class T>
size_t StructEntityType<T>::Deserialize(Entity* entity, const void* buffer)
{
    memcpy(entity, buffer, sizeof(T));
    return sizeof(T);
}


typedef std::vector<EntityType*> EntityTypeList;
void InitializeEntityTypes(EntityTypeList& entityTypes);

#endif

