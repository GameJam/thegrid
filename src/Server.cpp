#include "Server.h"

#include "Log.h"

Server::Client::Client(int id)
{
    m_id = id;
    m_state.m_id = id;
    m_state.m_test.resize(id + 1);
    m_state.m_test[id].x = 0;
    m_state.m_test[id].x = 0;
}

int Server::Client::GetId() const
{
    return m_id;
}

void Server::Client::GetTest(int& x, int& y)
{
    x = m_state.m_test[m_id].x;
    y = m_state.m_test[m_id].y;
}

void Server::Client::Update(Server& server)
{

    // 1. Execute orders
    
    
    // 2. Update state
    ClientList clients;
    server.GetClients(clients);

    for (size_t i = 0; i < clients.size(); ++i)
    {
        if (clients[i] == this)
        {
            continue;
        }
        
        Client* client = clients[i];
        if (static_cast<size_t>(client->m_id) >= m_state.m_test.size())
        {
            m_state.m_test.resize(client->m_id + 1);
        }
        client->GetTest(m_state.m_test[client->m_id].x, m_state.m_test[client->m_id].y);
    }

    
    // 3. Send new state
    size_t packetSize = m_state.GetSerializedSize() + sizeof(Protocol::StatePacket);
    char* buffer = new char[packetSize];

    Protocol::StatePacket* packet = reinterpret_cast<Protocol::StatePacket*>(buffer);
    packet->packetType = Protocol::PacketType_State;
    m_state.Serialize(packet->data);
    server.GetHost().SendPacket(m_id, 0, packet, packetSize);
    delete[] buffer;

}

void Server::Client::OnOrder(const Protocol::OrderPacket& order)
{
    
    LogDebug("Client %i ordered move to %i, %i", m_id, order.x, order.y);
    
    m_state.m_test[m_id].x = order.x;
    m_state.m_test[m_id].y = order.y;

}

Server::Server() : m_host(1)
{
    m_host.Listen(12345);

    m_mapSeed = 42;
}

Server::~Server()
{
    for (ClientMap::iterator i = m_clientMap.begin(); i != m_clientMap.end(); ++i)
    {
        delete i->second;
    }
    m_clientMap.clear();
}

void Server::Update()
{

    m_host.Service(this);

    for (ClientMap::iterator i = m_clientMap.begin(); i != m_clientMap.end(); ++i)
    {
        i->second->Update(*this);
    }

}

void Server::OnConnect(int peerId)
{
    m_clientMap[peerId] = new Client(peerId);

    Protocol::InitializeGamePacket initializeGame;
    initializeGame.packetType = Protocol::PacketType_InitializeGame;
    initializeGame.mapSeed = m_mapSeed;
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

Host& Server::GetHost()
{
    return m_host;
}

void Server::GetClients(ClientList& clients)
{
    clients.clear();
    for (ClientMap::iterator i = m_clientMap.begin(); i != m_clientMap.end(); ++i)
    {
        clients.push_back(i->second);
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

