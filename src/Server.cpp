#include "Server.h"

#include "Log.h"
#include "Map.h"

#include <SDL.h>

Server::Client::Client(int id, Server& server)
{

    m_id = id;
    m_map = &server.GetMap();
    m_random.Seed(SDL_GetTicks());
    EntityState& state = server.GetState();

    for (int i = 0; i < 3; ++i)
    {
        AgentEntity* agent = new AgentEntity();
        int stop = static_cast<int>(m_random.Generate(0, m_map->GetNumStops() - 1));
        agent->m_currentStop = stop;

        m_agents.push_back(agent);
        state.AddEntity(agent);
    }

}

int Server::Client::GetId() const
{
    return m_id;
}

void Server::Client::Update(Server& server)
{

    EntityState& state = server.GetState();

    // 1. Execute orders

    // FAKE ORDERS
    for (size_t i = 0; i < m_agents.size(); ++i)
    {
        AgentEntity* agent = m_agents[i];

        if (agent->m_targetStop == -1)
        {
            const std::vector<int>& neighbours = m_map->GetStop(agent->m_currentStop).children;
            agent->m_targetStop = neighbours[m_random.Generate(0, static_cast<int>(neighbours.size() - 1))];
            agent->m_arrivalTime = state.GetTime() + 2;
        }
    }

    
    // 2. Update state
    for (size_t i = 0; i < m_agents.size(); ++i)
    {
        AgentEntity* agent = m_agents[i];

        if (agent->m_targetStop != -1 && agent->m_arrivalTime < state.GetTime())
        {
            agent->m_currentStop = agent->m_targetStop;
            agent->m_targetStop = -1;
        }

    }

    /*
    ClientList clients;
    server.GetClients(clients);

    for (size_t i = 0; i < clients.size(); ++i)
    {

        Client* client = clients[i];
        if (client == this)
        {
            continue;
        }
        
        TestEntity* spiedEntity = NULL;

        stdext::hash_map<int, TestEntity*>::iterator iter = m_spiedTestEntities.find(client->GetId());
        if (iter == m_spiedTestEntities.end())
        {
            // new entity found!
            spiedEntity = new TestEntity();
            spiedEntity->clientId = client->GetId();
            m_state.AddEntity(spiedEntity);
            m_spiedTestEntities[client->GetId()] = spiedEntity;
        }
        else
        {
            spiedEntity = iter->second;
        }

        client->GetTest(spiedEntity->x, spiedEntity->y);

    }
    */


}

void Server::Client::OnOrder(const Protocol::OrderPacket& order)
{
    
    LogDebug("Client %i ordered move to %i, %i", m_id, order.x, order.y);
    
}

Server::Server() 
    : m_host(1), 
      m_globalState(&m_entityTypes)
{
    m_host.Listen(12345);

    m_time = 0;
    m_mapSeed = 42;
    m_gridSpacing = 150;
    m_xMapSize = m_gridSpacing * 9;
    m_yMapSize = m_gridSpacing * 6;

    InitializeEntityTypes(m_entityTypes);

    m_map.Generate(m_xMapSize, m_yMapSize, m_mapSeed);
}

Server::~Server()
{
    for (ClientMap::iterator i = m_clientMap.begin(); i != m_clientMap.end(); ++i)
    {
        delete i->second;
    }
    m_clientMap.clear();
}

void Server::Update(float deltaTime)
{

    m_time += deltaTime;
    m_globalState.SetTime(m_time);

    m_host.Service(this);

    for (ClientMap::iterator i = m_clientMap.begin(); i != m_clientMap.end(); ++i)
    {
        i->second->Update(*this);
    }

    for (ClientMap::iterator i = m_clientMap.begin(); i != m_clientMap.end(); ++i)
    {
        SendClientState(i->second->GetId());
    }



}

void Server::OnConnect(int peerId)
{
    m_clientMap[peerId] = new Client(peerId, *this);

    Protocol::InitializeGamePacket initializeGame;
    initializeGame.packetType = Protocol::PacketType_InitializeGame;
    initializeGame.mapSeed = m_mapSeed;
    initializeGame.gridSpacing = m_gridSpacing;
    initializeGame.xMapSize = m_xMapSize;
    initializeGame.yMapSize = m_yMapSize;

    m_host.SendPacket(peerId, 0, &initializeGame, sizeof(Protocol::InitializeGamePacket));
}

void Server::OnDisconnect(int peerId)
{
    ClientMap::iterator iter = m_clientMap.find(peerId);
    if (iter != m_clientMap.end())
    {
        delete iter->second;
        m_clientMap.erase(iter);
    }
}

void Server::OnPacket(int peerId, int channel, void* data, size_t size)
{

    if (size == 0)
    {
        return;
    }

    char* byteData = static_cast<char*>(data);
    Protocol::PacketType packetType = static_cast<Protocol::PacketType>(*byteData);

    Client* client = FindClient(peerId);

    switch (packetType)
    {
    case Protocol::PacketType_Order:
        if (size != sizeof(Protocol::OrderPacket))
        {
            LogError("Malformed order packet");
        }
        else if (client != NULL)
        {
            client->OnOrder(*static_cast<Protocol::OrderPacket*>(data));
        }
        break;

    default:
        LogDebug("Unrecognized packet: %i", packetType);
    }
    
}

float Server::GetTime() const
{
    return m_time;
}

void Server::GetClients(ClientList& clients)
{
    clients.clear();
    for (ClientMap::iterator i = m_clientMap.begin(); i != m_clientMap.end(); ++i)
    {
        clients.push_back(i->second);
    }
}

EntityState& Server::GetState()
{
    return m_globalState;
}

Map& Server::GetMap()
{
    return m_map;
}

Server::Client* Server::FindClient(int peerId)
{

    ClientMap::iterator iter = m_clientMap.find(peerId);
    if (iter == m_clientMap.end())
    {
        return NULL;
    }

    return iter->second;

}

void Server::SendClientState(int clientId)
{

    size_t dataSize = m_globalState.GetSerializedSize(clientId);
    size_t packetSize = dataSize + sizeof(Protocol::StatePacketHeader);
    char* buffer = new char[packetSize];

    Protocol::StatePacket* packet = reinterpret_cast<Protocol::StatePacket*>(buffer);
    packet->header.packetType = Protocol::PacketType_State;
    packet->header.dataSize = dataSize;
    m_globalState.Serialize(clientId, packet->data, dataSize);
    m_host.SendPacket(clientId, 0, packet, packetSize);
    delete[] buffer;

}
