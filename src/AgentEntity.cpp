#include "AgentEntity.h"

AgentEntity::AgentEntity()
{
    m_currentStop   = -1;
    m_targetStop    = -1;
    m_arrivalTime   = 0;
    m_hasIntel      = false;
    m_state         = State_Idle;
}
