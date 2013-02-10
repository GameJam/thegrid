#include "Server.h"

#include "Log.h"
#include "Map.h"
#include "BuildingEntity.h"
#include "PlayerEntity.h"

#include <SDL.h>
#include <algorithm>

static const float kServerTickRate      = 1.0f / 30.0f;
static const float kServerBroadcastRate = 1.0f;

Server::Client::Client(int id, Server& server)
{

    m_id = id;
    m_server = &server;
    m_map = &server.GetMap();
    m_state = &server.GetState();

    m_random.Seed(SDL_GetTicks());

    const int numAgents     = 3;
    const int numSafeHouses = 3;

    m_player = m_state->CreateEntity<PlayerEntity>();
    sprintf(m_player->m_name, "Mr. %c", 'Q' + (id % 10));
    m_player->m_clientId = id;

    for (int i = 0; i < numAgents; ++i)
    {
        AgentEntity* agent = m_state->CreateEntity<AgentEntity>(m_id);
        m_agents.push_back(agent);

        int stop = static_cast<int>(m_random.Generate(0, m_map->GetNumStops() - 1));
        agent->m_currentStop = stop;
    }

    for (int i = 0; i < numSafeHouses; ++i)
    {

        int offset = m_random.Generate(0, m_map->GetNumStops() - 1);
        int stopIndex = -1;

        // Note, can place two player's safe houses at the same location, but should
        // be ok for gameplay.
        for (int j = 0; j < m_map->GetNumStops(); ++j)
        {
            stopIndex = (j + offset) % m_map->GetNumStops();
            if (m_map->GetStop(stopIndex).structureType  == StructureType_None)
            {
                break;
            }
        }
        
        assert(stopIndex != -1);

        BuildingEntity* building = m_state->CreateEntity<BuildingEntity>(m_id);
        building->m_stop = stopIndex;
    }
    
}

int Server::Client::GetId() const
{
    return m_id;
}

void Server::Client::Update()
{

    if (m_player->m_eliminated)
    {
        return;
    }

    // Update lost agents
    for (AgentList::iterator i = m_agents.begin(); i != m_agents.end();)
    {
        if ((*i)->GetOwnerId() != m_id)
        {
            // Agent lost!
            i = m_agents.erase(i);
        }
        else
        {
            ++i;
        }
    }

    // Move agents
    for (size_t i = 0; i < m_agents.size(); ++i)
    {
        AgentEntity* agent = m_agents[i];

        if (agent->m_targetStop != -1 && agent->m_arrivalTime < m_state->GetTime())
        {
            agent->m_currentStop = agent->m_targetStop;
            agent->m_targetStop = -1;
        }

    }

    // Check for end game
    if (m_agents.size() == 0)
    {
        m_player->m_eliminated = true;
    }

    UpdateHackingStatus();

}

void Server::Client::OnOrder(const Protocol::OrderPacket& order)
{
    
    AgentEntity* agent = FindAgent(order.agentId);

    if (agent == NULL || m_player->m_eliminated)
    {
        return;
    }

    switch (order.order)
    {
    case Protocol::Order_MoveTo:
        {
            const std::vector<int>& neighbors = m_map->GetStop(agent->m_currentStop).children;
            if (agent->m_targetStop == -1 && std::find(neighbors.begin(), neighbors.end(), order.targetStop) != neighbors.end())
            {
                agent->m_targetStop = order.targetStop;
                agent->m_departureTime = m_state->GetTime();
                agent->m_arrivalTime = m_state->GetTime() + 1;
                agent->m_state = AgentEntity::State_Idle;
            }
        }
        break;

    case Protocol::Order_Capture:
        for (int i = 0; i < m_state->GetNumEntities(); ++i)
        {
            Entity* entity = m_state->GetEntity(i);
            if (entity->GetTypeId() == EntityTypeId_Agent && entity->GetOwnerId() != m_id)
            {
                AgentEntity* capturedAgent = static_cast<AgentEntity*>(entity);
                if (capturedAgent->m_currentStop == agent->m_currentStop)
                {
                    // Capture this agent!
                    int oldOwnerId = capturedAgent->GetOwnerId();
                    capturedAgent->SetOwnerId(m_id);
                    m_agents.push_back(capturedAgent);
                    m_server->SendNotification(m_id, Protocol::Notification_AgentCaptured, capturedAgent->GetId(), capturedAgent->m_currentStop, -1);
                    m_server->SendNotification(oldOwnerId, Protocol::Notification_AgentLost, -1, capturedAgent->m_currentStop, -1);
                    break;
                }
            }
        }
        break;

    case Protocol::Order_Hack:
        {
            StructureType structureType = m_map->GetStop(agent->m_currentStop).structureType;
            if (structureType != StructureType_None)
            {
                if (agent->m_state == AgentEntity::State_Hacking)
                {
                    agent->m_state = AgentEntity::State_Idle;
                }
                else
                {
                    agent->m_state = AgentEntity::State_Hacking;
                }
                break;
            }
        }
        break;

    case Protocol::Order_Stakeout:
        {
            if (agent->m_state == AgentEntity::State_Stakeout)
            {
                agent->m_state = AgentEntity::State_Idle;
            }
            else
            {
                agent->m_state = AgentEntity::State_Stakeout;
            }   
        }
        break;

    }
    
}

void Server::Client::UpdateHackingStatus()
{
    m_player->m_hackingBank   = false;
    m_player->m_hackingTower  = false;
    m_player->m_hackingPolice = false;
    for (size_t i = 0; i < m_agents.size(); ++i)
    {
        const AgentEntity* agent = m_agents[i];
        if (agent->m_state == AgentEntity::State_Hacking)
        {
            switch (m_map->GetStop(agent->m_currentStop).structureType)
            {   
            case StructureType_Bank:
                m_player->m_hackingBank = true;
                break;
            case StructureType_Tower:
                m_player->m_hackingTower = true;
                break;
            case StructureType_Police:
                m_player->m_hackingPolice = true;
                break;
            default:
                assert(0);
            }
        }
    }
}

AgentEntity* Server::Client::FindAgent(int agentId)
{
    for (size_t i = 0; i < m_agents.size(); ++i)
    {
        if (m_agents[i]->GetId() == agentId)
        {
            return m_agents[i];
        }
    }

    return NULL;
}


Server::Server() 
    : m_host(1), 
      m_globalState(&m_typeRegistry)
{
    int gamePort = 12345;
    m_host.Listen(gamePort);
    m_lanBroadcast.Initialize(Protocol::listenPort, gamePort);

    m_time                  = 0;
    m_timeSinceUpdate       = 0;
    m_timeSinceBroadcast    = 0;
    m_mapSeed               = 42;
    m_gridSpacing           = 150;
    m_xMapSize              = m_gridSpacing * 9;
    m_yMapSize              = m_gridSpacing * 6;

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

    m_timeSinceUpdate += deltaTime;
    m_timeSinceBroadcast += deltaTime;

    if (m_timeSinceBroadcast > kServerBroadcastRate)
    {
        m_lanBroadcast.BroadcastInfo();
        m_timeSinceBroadcast = 0.0f;
    }
    
    if (m_timeSinceUpdate > kServerTickRate)
    {

        m_timeSinceUpdate -= kServerTickRate;
        m_time += kServerTickRate;

        m_globalState.SetTime(m_time);

        m_host.Service(this);

        for (ClientMap::iterator i = m_clientMap.begin(); i != m_clientMap.end(); ++i)
        {
            i->second->Update();
        }

        for (ClientMap::iterator i = m_clientMap.begin(); i != m_clientMap.end(); ++i)
        {
            SendClientState(i->second->GetId());
        }

    }

}

void Server::OnConnect(int peerId)
{
    m_clientMap[peerId] = new Client(peerId, *this);

    Protocol::InitializeGamePacket initializeGame;
    initializeGame.time         = m_time;
    initializeGame.clientId     = peerId;
    initializeGame.packetType   = Protocol::PacketType_InitializeGame;
    initializeGame.mapSeed      = m_mapSeed;
    initializeGame.gridSpacing  = m_gridSpacing;
    initializeGame.xMapSize     = m_xMapSize;
    initializeGame.yMapSize     = m_yMapSize;
    
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

void Server::SendNotification(int peerId, Protocol::Notification notification, int agentId, int stop, int rail)
{

    Protocol::NotificationPacket packet;
    packet.packetType = Protocol::PacketType_Notification;
    packet.notification = notification;
    packet.agentId = agentId;
    packet.stop = stop;
    packet.rail = rail;
    m_host.SendPacket(peerId, 0, &packet, sizeof(packet));

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
