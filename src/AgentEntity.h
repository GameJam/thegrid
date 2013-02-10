#ifndef GAME_AGENT_ENTITY_H
#define GAME_AGENT_ENTITY_H

#include "Entity.h"

class AgentEntity : public Entity
{
public:

    AgentEntity();
    
    int GetCurrentStop() const;
    void SetCurrentStop(int stop);
    
private:
    
    int m_currentStop;

};

#endif
