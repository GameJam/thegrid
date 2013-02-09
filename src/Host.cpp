#include "Host.h"
#include "Log.h"

#include <enet/enet.h>

#include <algorithm>
#include <vector>

#include <assert.h>
#include <stdio.h>

static const int kMaxClients = 32;

struct Host::PrivateData
{

    ENetPeer* FindPeer(int peerId) const;
    void DeletePeer(ENetPeer* peer);

    typedef std::vector<ENetPeer*> PeerList;

    ENetHost*   m_host;
    PeerList    m_peers;

};

struct PeerData
{    
    PeerData(int id) : m_id(id) {}
    int m_id;
};


Host::Host(int numChannels)
{
    m_data = new PrivateData;
    m_data->m_host = NULL;

    m_nextPeerId = 1;
    m_numChannels = numChannels;
}

Host::~Host()
{
    Destroy();
    delete m_data;
}

void Host::Service(Handler* handler)
{
    
    if (m_data->m_host != NULL)
    {

        ENetEvent event;
        while (enet_host_service(m_data->m_host, &event, 0) > 0)
        {
    
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
                {
                    LogDebug("Peer connected from %x:%u", 
                             event.peer->address.host, event.peer->address.port);

                    int peerId = m_nextPeerId;
                    ++m_nextPeerId;

                    event.peer->data = new PeerData(peerId);
                    m_data->m_peers.push_back(event.peer);

                    if (handler != NULL)
                    {
                        handler->OnConnect(peerId);
                    }
                }
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                {
                    int peerId = static_cast<PeerData*>(event.peer->data)->m_id;

                    LogDebug("Peer %i disconnected", peerId);

                    if (handler != NULL)
                    {
                        handler->OnDisconnect(peerId);
                    }
                    m_data->DeletePeer(event.peer);
                }
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                if (handler != NULL)
                {
                    int peerId = static_cast<PeerData*>(event.peer->data)->m_id;
                    handler->OnPacket(peerId, event.channelID, event.packet->data, event.packet->dataLength);
                }
                enet_packet_destroy(event.packet);
                break;

            }

        }

    }

}

bool Host::SendPacket(int peerId, int channel, void* data, size_t size)
{

    bool result = false;
    ENetPeer* eNetPeer = m_data->FindPeer(peerId);

    if (eNetPeer != NULL)
    {
        ENetPacket* packet = enet_packet_create(data, size, ENET_PACKET_FLAG_RELIABLE);
        result = enet_peer_send(eNetPeer, channel, packet) == 0;
    }

    return result;

}

bool Host::Listen(int port)
{

    Destroy();

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    m_data->m_host = enet_host_create(&address, kMaxClients, m_numChannels, 0, 0);

    if (m_data->m_host == NULL)
    {
        LogError("Failed to bind to port %d!", port);
        return false;
    }
    else
    {
        return true;
    }

}

bool Host::Connect(const char* hostName, int port)
{

    Destroy();

    m_data->m_host = enet_host_create(NULL, 1, m_numChannels, 0, 0);
    if (m_data->m_host == NULL)
    {
        LogError("Failed to create host!");
        return false;
    }

    ENetAddress address;
    if (enet_address_set_host(&address, hostName) != 0)
    {
        LogError("Failed to set host!");
        Destroy();
        return false;
    }
    address.port = port;

    ENetPeer* peer = enet_host_connect(m_data->m_host, &address, m_numChannels, 0);
    if (peer == NULL)
    {
        LogError("Failed to connect!");
        Destroy();
        return false;
    }

    m_data->m_peers.push_back(peer);
    return true;

}

void Host::Destroy()
{

    while(!m_data->m_peers.empty())
    {
        enet_peer_disconnect(m_data->m_peers[0], 0);
        m_data->DeletePeer(m_data->m_peers[0]);
    }
       
    if (m_data->m_host != NULL)
    {
        enet_host_flush(m_data->m_host);
        enet_host_destroy(m_data->m_host);
        m_data->m_host = NULL;
    }

}

void Host::Initialize()
{
    enet_initialize();
}

void Host::Shutdown()
{
    enet_deinitialize();
}

ENetPeer* Host::PrivateData::FindPeer(int peerId) const
{
    
    for (size_t i = 0; i < m_peers.size(); ++i)
    {
        if (static_cast<PeerData*>(m_peers[i]->data)->m_id == peerId)
        {
            return m_peers[i];
        }
    }

    return NULL;

}


void Host::PrivateData::DeletePeer(ENetPeer* peer)
{
    
    PeerList::iterator iter = std::find(m_peers.begin(), m_peers.end(), peer);
    if (iter != m_peers.end())
    {
        m_peers.erase(iter);
    }

    delete static_cast<PeerData*>(peer->data);
    peer->data = NULL;

}