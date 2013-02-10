#ifndef GAME_AGENT_ENTITY_H
#define GAME_AGENT_ENTITY_H

#include "Entity.h"

class AgentEntity : public Entity
{

public:

    enum { TypeId = EntityTypeId_Agent };

    enum State
    {
        State_Idle,
        State_Hacking,
        State_Stakeout,
    };

    AgentEntity();
    
    int     m_currentStop;
    int     m_intel;
    State   m_state;
    
    // Movement hack
    int     m_targetStop;
    float   m_departureTime;
    float   m_arrivalTime;

};

#endif
