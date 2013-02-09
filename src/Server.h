#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include "Host.h"

#include "Protocol.h"
#include "ClientWorldState.h"

#include <hash_map>

class Server : public Host::Handler
{
public:


    class Client
    {
    public:
        Client(int id);

        int GetId() const;

        void Update(Server& server);

        void OnOrder(const Protocol::OrderPacket& order);

        void GetTest(int& x, int& y);

    private:
        int m_id;
        ClientWorldState m_state;
    };

    typedef std::vector<Client*> ClientList;

    Server();
    virtual ~Server();

    void Update();

    virtual void OnConnect(int peerId);
    virtual void OnDisconnect(int peerId);
    virtual void OnPacket(int peerId, int channel, void* data, size_t size);

    Host& GetHost();
    void GetClients(ClientList& clients);

private:
    
    Client* FindClient(int peerId);

    typedef stdext::hash_map<int, Client*> ClientMap;

    Host        m_host;
    ClientMap   m_clientMap;

};

#endif
