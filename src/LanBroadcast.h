#ifndef GAME_LAN_BROADCAST_H
#define GAME_LAN_BROADCAST_H

class LanBroadcast
{

public:

    LanBroadcast();
    ~LanBroadcast();

    bool Initialize(int port);
    void Shutdown();

    bool BroadcastInfo();

private:

    int             m_socket;
    char            m_serverName[64];
    int             m_port;

};

#endif