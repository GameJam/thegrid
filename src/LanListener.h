#ifndef GAME_LAN_LISTENER_H
#define GAME_LAN_LISTENER_H

#include <time.h>

class LanListener
{

public:

    static const int    s_maxServerName = 64;
    static const int    s_maxServers    = 64;
    static const time_t s_serverTimeout = 4;

    struct Server
    {
        char            name[s_maxServerName];
        unsigned long   ip;
        int             port;
        time_t          time;
    };

    LanListener();

    bool Initialize(int port);
    void Service();

private:

    void AddServer(const char* name, unsigned long ip, int port, time_t time);
    void RemoveOldServers(time_t time);

private:

    int             m_port;
    int             m_socket;

    int             m_numServers;
    Server          m_server[s_maxServers];

};

#endif