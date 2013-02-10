#ifndef GAME_LAN_LISTENER_H
#define GAME_LAN_LISTENER_H

class LanListener
{

public:

    LanListener();

    bool Initialize(int port);
    void Service();

private:

    static const int s_maxServerName = 64;
    static const int s_maxServers    = 64;

    struct Server
    {
        char name[s_maxServerName];
    };

    int             m_port;
    int             m_socket;

    int             m_numServers;
    Server          m_server[s_maxServers];

};

#endif