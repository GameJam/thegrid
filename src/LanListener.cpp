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

    return true;
}

void LanListener::Service()
{

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

        int a = 0;

    }

}