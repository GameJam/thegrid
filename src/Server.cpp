#include "Server.h"

#include "Log.h"

Server::Client::Client(int id)
{
    m_id = id;
}

void Server::Client::Update()
{

    // 1. Execute orders
    // 2. Update state

}

void Server::Client::OnOrder(const Protocol::OrderPacket& order)
{
    LogDebug("Client %i ordered move to %i, %i", m_id, order.x, order.y);
}

Server::Server() : m_host(1)
{
    m_host.Listen(12345);
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
        i->second->Update();
    }

}

void Server::OnConnect(int peerId)
{
    m_clientMap[peerId] = new Client(peerId);
    m_host.SendPacket(peerId, 0, "Hello", sizeof("Hello") + 1);
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

