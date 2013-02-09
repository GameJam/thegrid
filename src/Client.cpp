#include "Client.h"

#include <stdio.h>

Client::Client() : m_host(1)
{
    m_serverId = -1;
}

void Client::Connect(const char* hostName, int port)
{
    m_host.Connect(hostName, port);
}

void Client::Update()
{
    m_host.Service(this);
}

void Client::OnConnect(int peerId)
{
    m_serverId = peerId;    
}

void Client::OnDisconnect(int peerId)
{

}

void Client::OnPacket(int peerId, int channel, void* data, size_t size)
{
    if (peerId == m_serverId)
    {
        printf("Server says: %s", data);
    }
}
