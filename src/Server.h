#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include "Host.h"

#include "Protocol.h"
#include "EntityState.h"
#include "Entity.h"
#include "EntityType.h"
#include "Map.h"
#include "AgentEntity.h"
#include "Random.h"

#include <hash_map>

class Map;

class Server : public Host::Handler
{
public:


    class Client
    {
    public:
        Client(int id, Server& server);

        int GetId() const;

        void Update(Server& server);

        void OnOrder(const Protocol::OrderPacket& order);

    private:
        
        typedef std::vector<AgentEntity*> AgentList;

        int                 m_id;
        Map*                m_map;
        Random              m_random;
        AgentList           m_agents;

    };

    typedef std::vector<Client*> ClientList;

    Server();
    virtual ~Server();

    void Update(float deltaTime);

    virtual void OnConnect(int peerId);
    virtual void OnDisconnect(int peerId);
    virtual void OnPacket(int peerId, int channel, void* data, size_t size);

    float GetTime() const;
    void GetClients(ClientList& clients);

    EntityState& GetState();
    Map& GetMap();

private:
    
    Client* FindClient(int peerId);
    void SendClientState(int peerId);

    typedef stdext::hash_map<int, Client*> ClientMap;
    Host            m_host;
    ClientMap       m_clientMap;
    EntityTypeList  m_entityTypes;
    EntityState     m_globalState;
    Map             m_map;
    float           m_time;

    int             m_mapSeed;
    int             m_gridSpacing;
    int             m_xMapSize;
    int             m_yMapSize;


};

#endif
