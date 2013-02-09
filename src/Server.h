#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include "Host.h"

#include <hash_map>

class Server : public Host::Handler
{
public:
    
    class Client
    {
    public:
        Client(int id);

    private:
        int m_id;
    };

    Server();
    virtual ~Server();

    void Update();

    virtual void OnConnect(int peerId);
    virtual void OnDisconnect(int peerId);
    virtual void OnPacket(int peerId, int channel, void* data, size_t size);

private:
    
    typedef stdext::hash_map<int, Client*> ClientMap;

    Host        m_host;
    ClientMap   m_clientMap;

};

#endif
