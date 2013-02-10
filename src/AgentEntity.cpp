#include "AgentEntity.h"

AgentEntity::AgentEntity() : Entity(EntityTypeId_Agent)
{
    m_currentStop   = -1;
    m_targetStop    = -1;
    m_arrivalTime   = 0;
    m_hasIntel      = false;
}
