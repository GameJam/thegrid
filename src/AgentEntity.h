#ifndef GAME_AGENT_ENTITY_H
#define GAME_AGENT_ENTITY_H

#include "Entity.h"

class AgentEntity : public Entity
{
public:

    AgentEntity();
    
    int m_currentStop;
    
    // Movement hack
    int m_targetStop;
    float m_arrivalTime;

};

#endif
