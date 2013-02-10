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
#include "LanBroadcast.h"

#include <hash_map>

class Map;
class PlayerEntity;

class Server : public Host::Handler
{
public:


    class Client
    {
    public:
        Client(int id, Server& server);

        int GetId() const;

        void Update();

        void OnOrder(const Protocol::OrderPacket& order);

        void UpdateHackingStatus();
        void CheckForStakeout(AgentEntity* agent);

    private:

        AgentEntity* FindAgent(int agentId);
        
        typedef std::vector<AgentEntity*> AgentList;

        int                 m_id;
        Server*             m_server;
        Map*                m_map;
        EntityState*        m_state;
        Random              m_random;
        AgentList           m_agents;
        PlayerEntity*       m_player;

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

    void SendNotification(int peerId, Protocol::Notification notification, int agentId, int stop, int rail);

private:
    
    Client* FindClient(int peerId);
    void SendClientState(int peerId);

    typedef stdext::hash_map<int, Client*> ClientMap;

    LanBroadcast        m_lanBroadcast;
    Host                m_host;
    ClientMap           m_clientMap;
    EntityTypeRegistry  m_typeRegistry;
    EntityState         m_globalState;
    Map                 m_map;
    float               m_time;
    float               m_timeSinceUpdate;
    float               m_timeSinceBroadcast;

    int                 m_mapSeed;
    int                 m_gridSpacing;
    int                 m_xMapSize;
    int                 m_yMapSize;


};

#endif
