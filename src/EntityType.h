#ifndef GAME_ENTITY_TYPE_H
#define GAME_ENTITY_TYPE_H

#include <assert.h>
#include <vector>


enum EntityTypeId;
class Entity;

class EntityType
{

public:

    EntityTypeId GetTypeId();

    virtual Entity* Create(int entityId)=0;
    virtual size_t GetSerializedSize(Entity* entity) const=0;
    virtual size_t Serialize(const Entity* entity, void *buffer) const=0;
    virtual size_t Deserialize(Entity* entity, const void* buffer)=0;

protected:

    EntityTypeId    m_typeId;

};

template <class T>
class StructEntityType : public EntityType
{

public:

    StructEntityType();

    virtual Entity* Create(int entityId);
    virtual size_t GetSerializedSize(Entity* entity) const;
    virtual size_t Serialize(const Entity* entity, void *buffer) const;
    virtual size_t Deserialize(Entity* entity, const void* buffer);


};

template<class T>
StructEntityType<T>::StructEntityType()
{
    m_typeId = EntityTypeId(T::TypeId);
}

template<class T>
Entity* StructEntityType<T>::Create(int entityId)
{
    T* entity = Entity::CreateEntity<T>(entityId, m_typeId);
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

#endif

