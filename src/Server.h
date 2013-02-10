#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include "Host.h"

#include "Protocol.h"
#include "ClientWorldState.h"
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
        Client(int id, Map* map, EntityTypeList* entityTypes);

        int GetId() const;

        void Update(Server& server);

        void OnOrder(const Protocol::OrderPacket& order);

        const ClientWorldState& GetState() const;

    private:
        
        typedef std::vector<AgentEntity*> AgentList;

        int                 m_id;
        Map*                m_map;
        Random              m_random;
        AgentList           m_agents;

        ClientWorldState    m_state;
        TestEntity*         m_testEntity;
        stdext::hash_map<int, TestEntity*> m_spiedTestEntities;

    };

    typedef std::vector<Client*> ClientList;

    Server();
    virtual ~Server();

    void Update();

    virtual void OnConnect(int peerId);
    virtual void OnDisconnect(int peerId);
    virtual void OnPacket(int peerId, int channel, void* data, size_t size);

    Host& GetHost();
    void GetClients(ClientList& clients);

private:
    
    Client* FindClient(int peerId);
    void SendClientState(const ClientWorldState& state);


    typedef stdext::hash_map<int, Client*> ClientMap;
    Host            m_host;
    ClientMap       m_clientMap;
    EntityTypeList  m_entityTypes;
    Map             m_map;

    int             m_mapSeed;
    int             m_gridSpacing;
    int             m_xMapSize;
    int             m_yMapSize;

};

#endif
