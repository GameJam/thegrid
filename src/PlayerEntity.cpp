#include "PlayerEntity.h"

PlayerEntity::PlayerEntity()
{
    m_hackingBank   = false;
    m_hackingTower  = false;
    m_hackingPolice = false;
    m_eliminated    = false;
    m_clientId      = -1;
    m_name[0] = 0;
}
