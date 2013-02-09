#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include "Host.h"

class Client : Host::Handler
{
public:

    Client();

    void Connect(const char* hostName, int port);

    void Update();

    virtual void OnConnect(int peerId);
    virtual void OnDisconnect(int peerId);
    virtual void OnPacket(int peerId, int channel, void* data, size_t size);

private:

    Host    m_host;
    int     m_serverId;
};

#endif
