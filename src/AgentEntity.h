#ifndef GAME_AGENT_ENTITY_H
#define GAME_AGENT_ENTITY_H

#include "Entity.h"

class AgentEntity : public Entity
{

public:

    enum { TypeId = EntityTypeId_Agent };

    enum State {
        State_Idle,
        State_Hacking,
    };

    AgentEntity();
    
    int     m_currentStop;
    bool    m_hasIntel;
    State   m_state;
    
    // Movement hack
    int     m_targetStop;
    float   m_departureTime;
    float   m_arrivalTime;

};

#endif
