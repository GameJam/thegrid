#include "Server.h"


Server::Client::Client(int id)
{
    m_id = id;
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

}
