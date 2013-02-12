#include "Server.h"

#include "Log.h"
#include "Map.h"
#include "BuildingEntity.h"
#include "PlayerEntity.h"

#include <SDL.h>
#include <algorithm>
#include <time.h>

static const float kServerTickRate      = 1.0f / 30.0f;
static const float kServerBroadcastRate = 1.0f;

static const float kIntelHackTime       = 5.0f;

Server::Client::Client(int id, Server& server)
{

    m_id = id;
    m_server = &server;
    m_map = &server.GetMap();
    m_state = &server.GetState();

    m_random.Seed(SDL_GetTicks());

    const int numAgents     = 5;
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
            if (agent->m_intel != -1)
            {
                m_server->GetIntel(agent->m_intel).m_stop = agent->m_currentStop;
            }
            CheckForStakeout(agent);
        }

    }

    // Hacking
    if (m_player->m_hackingTower && m_player->m_nextIntelPing < m_state->GetTime())
    {
        m_player->m_lastIntelFound = m_server->PingIntel(m_id, m_player->m_lastIntelFound);
        m_player->m_nextIntelPing = m_state->GetTime() + kIntelHackTime;
    }

    UpdateCounts();

    // Check for end game
    if (m_agents.size() == 0 || m_player->m_numSafeHouses == 0)
    {
        m_player->m_eliminated = true;
    }

    UpdateHackingStatus();


}

void Server::Client::CheckForStakeout(AgentEntity* agent)
{
    bool spotted = false;

    // Check if the location of the agent is being staked out.
    int stop = agent->m_currentStop;
    int index = 0;
    const AgentEntity* testAgent;
    while (m_state->GetNextEntityWithType(index, testAgent))
    {
        if (testAgent->m_state == AgentEntity::State_Stakeout &&
            testAgent->m_currentStop == stop &&
            testAgent->GetOwnerId() != m_id)
        {
            int id = testAgent->GetOwnerId();
            m_server->SendNotification(id, Protocol::Notification_AgentSpotted, agent->GetId(), agent->m_currentStop, -1);
            spotted = true;

        }
    }

    if (spotted)
    {
        NotifyCrime(agent->GetId(), agent->m_currentStop);
    }

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

                int line = m_map->GetLineBetween(agent->m_currentStop, agent->m_targetStop);
                assert(line != -1);
                m_server->OnLineUsed(m_id, line);
            }
        }
        break;

    case Protocol::Order_Capture:
        {
            bool agentCaptured = false;

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
                        if (capturedAgent->m_intel != -1)
                        {
                            IntelData& intelData = m_server->GetIntel(capturedAgent->m_intel);
                            intelData.m_agentId = -1;
                            intelData.m_owner = -1;
                            capturedAgent->m_intel = -1;
                        }
                        m_agents.push_back(capturedAgent);
                        m_server->SendNotification(m_id, Protocol::Notification_AgentCaptured, capturedAgent->GetId(), capturedAgent->m_currentStop, -1);
                        m_server->SendNotification(oldOwnerId, Protocol::Notification_AgentLost, -1, capturedAgent->m_currentStop, -1);
                        agentCaptured = true;
                        break;
                    }
                }
            }

            if (!agentCaptured)
            {
                m_server->NotifyCrime(agent->GetId(), agent->m_currentStop);
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
        if (agent->m_state == AgentEntity::State_Stakeout)
        {
            agent->m_state = AgentEntity::State_Idle;
        }
        else
        {
            agent->m_state = AgentEntity::State_Stakeout;
        }   
        break;

    case Protocol::Order_Infiltrate:
        Infiltrate(agent);
        break;

    case Protocol::Order_Intel:
        if (agent->m_intel == -1)
        {
            TakeIntel(agent);
        }
        else
        {
            DropIntel(agent);
        }
        break;

    }
    
}

void Server::Client::Infiltrate(AgentEntity* agent)
{
    // Check if there is a safe house at this stop.

    bool infiltrated = false;
    int stop = agent->m_currentStop;
    int id = agent->GetOwnerId();
    int index = 0;
    BuildingEntity* structure;
    while (m_state->GetNextEntityWithType(index, structure))
    {
        if (structure->m_stop == stop && structure->GetOwnerId() != id && !structure->m_raided)
        {
            structure->m_raided = true;
            structure->m_numIntels = 0;
            m_server->SendNotification(id, Protocol::Notification_HouseDestroyed, agent->GetId(), agent->m_currentStop, -1);
            m_server->SendNotification(structure->GetOwnerId(), Protocol::Notification_HouseDestroyed, agent->GetId(), agent->m_currentStop, -1);
            infiltrated = true;

            // Re-drop intel
            for (int i = 0; i < m_server->GetNumIntels(); ++i)
            {
                IntelData& intelData = m_server->GetIntel(i);
                if (intelData.m_inHouse && intelData.m_stop == stop && intelData.m_owner != id)
                {
                    intelData.m_owner = -1;
                    intelData.m_inHouse = false;
                }
            }
            break;
        }
    }

    if (!infiltrated)
    {
        m_server->NotifyCrime(agent->GetId(), agent->m_currentStop);
    }

}

void Server::Client::TakeIntel(AgentEntity* agent)
{
    if (agent->m_intel != -1)
    {
        return;
    }

    int intel = m_server->GetIntelAtStop(agent->m_currentStop);
    if (intel != -1)
    {
        int clientId = agent->GetOwnerId();

        IntelData& intelData = m_server->GetIntel(intel);
        intelData.m_agentId = agent->GetId();
        intelData.m_owner = clientId;
        m_server->SendNotification(clientId, Protocol::Notification_IntelCaptured, agent->GetId(), agent->m_currentStop, -1);
        agent->m_intel = intel;
    }
    else
    {
        m_server->NotifyCrime(agent->GetId(), agent->m_currentStop);
    }
}

void Server::Client::DropIntel(AgentEntity* agent)
{
    if (agent->m_intel == -1)
    {
        return;
    }

    int index = 0;
    BuildingEntity* safeHouse;
    while (m_state->GetNextEntityWithType(index, safeHouse))
    {
        if ((safeHouse->m_stop == agent->m_currentStop) &&
            (safeHouse->GetOwnerId() == agent->GetOwnerId()))
        {
            ++safeHouse->m_numIntels;
            IntelData& intelData = m_server->GetIntel(agent->m_intel);
            assert(intelData.m_stop == agent->m_currentStop);
            intelData.m_inHouse = true;
            intelData.m_agentId = -1;
            agent->m_intel = -1;
            break;
        }
    }

}

void Server::Client::NotifyCrime(int agentId, int stop)
{
    if (m_player->m_hackingPolice)
    {
        m_server->SendNotification(m_id, Protocol::Notification_CrimeDetected, agentId, stop, -1);
    }
}

PlayerEntity* Server::Client::GetPlayer()
{
    return m_player;
}

void Server::Client::UpdateHackingStatus()
{
    bool wasHackingTower = m_player->m_hackingTower;

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

    if (!wasHackingTower && m_player->m_hackingTower)
    {
        m_player->m_lastIntelFound = -1;
        m_player->m_nextIntelPing = m_state->GetTime() + kIntelHackTime;
    }
}

void Server::Client::UpdateCounts()
{
    m_player->m_numAgents = 0;
    m_player->m_numSafeHouses = 0;

    int numEntities = m_state->GetNumEntities();
    for (int i = 0; i < numEntities; ++i)
    {
        Entity* entity = m_state->GetEntity(i);    
        if (entity->GetOwnerId() == m_id)
        {
            if (entity->GetTypeId() == EntityTypeId_Building && !static_cast<BuildingEntity*>(entity)->m_raided)
            {
                ++m_player->m_numSafeHouses;
            }
            else if (entity->GetTypeId() == EntityTypeId_Agent)
            {
                ++m_player->m_numAgents;
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
    const int numIntels     = 5;
    const int gamePort      = 12345;

    m_random.Seed(SDL_GetTicks());

    m_host.Listen(gamePort);
    m_lanBroadcast.Initialize(Protocol::listenPort, gamePort);

    m_time                  = 0;
    m_timeSinceUpdate       = 0;
    m_timeSinceBroadcast    = 0;
    m_mapSeed               = static_cast<int>(time(NULL));
    m_gridSpacing           = 150;
    m_xMapSize              = m_gridSpacing * 9;
    m_yMapSize              = m_gridSpacing * 6;

    m_map.Generate(m_xMapSize, m_yMapSize, m_mapSeed);

    // Generate intel
    m_intelList.resize(numIntels);
    for (int i = 0; i < numIntels; ++i)
    {
        m_intelList[i].m_stop = -1;        
        m_intelList[i].m_agentId = -1;
        m_intelList[i].m_owner = -1;
        m_intelList[i].m_inHouse = false;
    }

    for (int i = 0; i < numIntels; ++i)
    {        
        int stop = -1;
        for (int attempt = 0; attempt < 32; ++attempt)
        {
            stop = m_random.Generate(0, m_map.GetNumStops() - 1);
            if (GetIntelAtStop(stop) == -1)
            {
                break;
            }
        }

        m_intelList[i].m_stop = stop;
    }

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

    // Check intel end game condition
    for (ClientMap::iterator i = m_clientMap.begin(); i != m_clientMap.end(); ++i)
    {
        i->second->GetPlayer()->m_numIntels = 0;
    }

    int maxIntels = 0;
    int clientWithIntel = -1;
    for (int i = 0; i < GetNumIntels(); ++i)
    {
        if (m_intelList[i].m_inHouse)
        {
            Client* client = FindClient(m_intelList[i].m_owner);
            if (client)
            {
                ++client->GetPlayer()->m_numIntels;
                maxIntels = std::max(maxIntels, client->GetPlayer()->m_numIntels);
                clientWithIntel = client->GetId();
            }            
        }
    }
    
    if (maxIntels == m_intelList.size())
    {
        for (ClientMap::iterator i = m_clientMap.begin(); i != m_clientMap.end(); ++i)
        {
            if (i->first != clientWithIntel)
            {
                i->second->GetPlayer()->m_eliminated = true;
            }
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
    initializeGame.totalNumIntels    = static_cast<int>(m_intelList.size());
    
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

void Server::NotifyCrime(int agentId, int stop)
{
    for (ClientMap::iterator i = m_clientMap.begin(); i != m_clientMap.end(); ++i)
    {
        Client* client = i->second;
        client->NotifyCrime(agentId, stop);
    }
}

int Server::GetNumIntels() const
{
    return static_cast<int>(m_intelList.size());
}

Server::IntelData& Server::GetIntel(int intel)
{
    return m_intelList[intel];
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

void Server::SendNotification(int peerId, Protocol::Notification notification, int agentId, int stop, int line)
{

    Protocol::NotificationPacket packet;
    packet.packetType = Protocol::PacketType_Notification;
    packet.notification = notification;
    packet.agentId = agentId;
    packet.stop = stop;
    packet.line = line;
    m_host.SendPacket(peerId, 0, &packet, sizeof(packet));

}

void Server::OnLineUsed(int clientId, int lineId)
{
    int index = 0;
    const PlayerEntity* player;
    while (m_globalState.GetNextEntityWithType(index, player))
    {
        if (player->m_clientId != clientId && player->m_hackingBank)
        {
            SendNotification(player->m_clientId, Protocol::Notification_LineUsed, -1, -1, lineId);
        }
    }
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

int Server::GetIntelAtStop(int stop)
{
    int result = 0;
    for (size_t i = 0; i < m_intelList.size(); ++i)
    {
        if (m_intelList[i].m_stop == stop && !m_intelList[i].m_inHouse && m_intelList[i].m_agentId == -1)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int Server::PingIntel(int clientId, int lastPinged)
{    
    
    int intel = (lastPinged + 1) % static_cast<int>(m_intelList.size());

    for (size_t i = 0; i < m_intelList.size(); ++i)
    {
        if (m_intelList[intel].m_owner != clientId && !m_intelList[intel].m_inHouse)
        {
            SendNotification(clientId, Protocol::Notification_IntelDetected, -1, m_intelList[intel].m_stop, -1);
            return intel;
        }

        intel = (intel + 1) % static_cast<int>(m_intelList.size());

    }

    return -1;

}