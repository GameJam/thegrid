#include "LanBroadcast.h"

#include <winsock2.h>
#include <ws2tcpip.h>

static void ConvertAddress(unsigned long ip, int port, sockaddr_in& address)
{
    address.sin_port              = htons(port);
    address.sin_addr.S_un.S_addr  = htonl(0x7F000001);
    address.sin_family            = AF_INET;
}

LanBroadcast::LanBroadcast()
{
    m_serverName[0] = 0;
}

bool LanBroadcast::Initialize(int port)
{

    m_port = port;

    m_socket = (int)socket(AF_INET,SOCK_DGRAM, 0);
    if (m_socket == -1)
    {
        return false;
    }

    static int so_broadcast = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, (const char*)&so_broadcast, sizeof(so_broadcast)) == -1)
    {
        return false;
    }

#ifdef WIN32
    DWORD length = sizeof(m_serverName) - 1;
    m_serverName[0] = 0;
    GetComputerNameA(m_serverName, &length);
#endif

    return true;
}

bool LanBroadcast::BroadcastInfo()
{

    char packet[ sizeof(m_serverName) ];
    memcpy(packet, m_serverName, sizeof(m_serverName));

    sockaddr_in address;
    address.sin_port            = htons(m_port);
    address.sin_family          = AF_INET;
    address.sin_addr.s_addr     = INADDR_BROADCAST;

    int result = sendto(m_socket, packet, sizeof(packet),
        0, (const sockaddr*)&address, sizeof(address));
    return result != SOCKET_ERROR;

}
