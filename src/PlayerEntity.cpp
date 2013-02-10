#include "PlayerEntity.h"

PlayerEntity::PlayerEntity() : Entity(EntityTypeId_Player)
{
    m_hackingBank   = false;
    m_hackingTower  = false;
    m_hackingPolice = false;
    m_name[0] = 0;
}
