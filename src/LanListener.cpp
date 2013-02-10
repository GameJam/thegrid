#include "LanListener.h"

#include <winsock2.h>
#include <ws2tcpip.h>

LanListener::LanListener()
{
    m_numServers    = 0;
    m_port          = 0;
}

bool LanListener::Initialize(int port)
{
    m_port  = port;
    m_socket = (int)socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in address;
    address.sin_port                = htons(m_port);
    address.sin_family              = AF_INET;
    address.sin_addr.s_addr         = INADDR_ANY;

    if (bind(m_socket, (struct sockaddr*)&address, sizeof(address)) == -1)
    {
        return false;
    }

    int so_reuseaddr = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&so_reuseaddr, sizeof(so_reuseaddr)) == -1)
    {
        return false;
    }

    return true;
}

void LanListener::Service()
{

    time_t currentTime = time(NULL);

    RemoveOldServers(currentTime);

    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(m_socket, &readSet);

    timeval timeVal;
    timeVal.tv_sec = 0;
    timeVal.tv_usec = 0;

    int result = select(0, &readSet, NULL, NULL, &timeVal);

    if (result > 0)
    {
        sockaddr_in address;
        int addressSize = sizeof(address);

        char buffer[1024];
        result = recvfrom(m_socket, static_cast<char*>(buffer), sizeof(buffer), 0,
            (sockaddr*)&address, &addressSize);

        unsigned long ip   = ntohl(address.sin_addr.S_un.S_addr);
        int           port = ntohs(address.sin_port);

        AddServer(buffer, ip, port, currentTime);

    }

}

void LanListener::AddServer(const char* name, unsigned long ip, int port, time_t time)
{

    // Check if this server is already in our list.
    for (int i = 0; i < m_numServers; ++i)
    {
        if (m_server[i].ip == ip)
        {
            m_server[i].port = port;
            m_server[i].time = time;
            strncpy(m_server[i].name, name, s_maxServerName);
            return;
        }
    }

    // Add a new server.
    if (m_numServers < s_maxServers)
    {
        m_server[m_numServers].ip   = ip;
        m_server[m_numServers].port = port;
        m_server[m_numServers].time = time;
        strncpy(m_server[m_numServers].name, name, s_maxServerName);
    }

}

void LanListener::RemoveOldServers(time_t time)
{
    for (int i = 0; i < m_numServers; ++i)
    {
        if (time - m_server[i].time > s_serverTimeout)
        {
            --m_numServers;
            m_server[i] = m_server[m_numServers];
            --i;
        }
    }
}