#ifndef GAME_HOST_H
#define GAME_HOST_H

class Host
{

public:
    
    class Handler
    {
    public:
        virtual void OnConnect(int peerId)=0;
        virtual void OnDisconnect(int peerId)=0;
        virtual void OnPacket(int peerId, int channel, void* data, size_t size)=0;
    };

    Host(int numChannels);
    ~Host();

    void Service(Handler* handler);
    bool SendPacket(int peerId, int channel, void* data, size_t size);

    bool Listen(int port);
    bool Connect(const char* hostName, int port);

    void Destroy();

    static void Initialize();
    static void Shutdown();

private:

    struct PrivateData;

    int             m_numChannels;
    int             m_nextPeerId;
    PrivateData*    m_data;

};

#endif
