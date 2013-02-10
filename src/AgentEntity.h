#ifndef GAME_AGENT_ENTITY_H
#define GAME_AGENT_ENTITY_H

#include "Entity.h"

class AgentEntity : public Entity
{

public:

    enum { TypeId = EntityTypeId_Agent };

    AgentEntity();
    
    int     m_currentStop;
    bool    m_hasIntel;
    
    // Movement hack
    int     m_targetStop;
    float   m_departureTime;
    float   m_arrivalTime;

};

#endif
