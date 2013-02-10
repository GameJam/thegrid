#include "AgentEntity.h"

AgentEntity::AgentEntity() : Entity(EntityTypeId_Agent)
{
    m_currentStop = -1;
}

int AgentEntity::GetCurrentStop() const
{
    return m_currentStop;
}

void AgentEntity::SetCurrentStop(int stop)
{
    m_currentStop = stop;
}
