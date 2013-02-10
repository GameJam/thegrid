#ifndef GAME_PROTOCOL_H
#define GAME_PROTOCOL_H

namespace Protocol
{

const int listenPort = 12347;

enum PacketType
{
    PacketType_InitializeGame,
    PacketType_Order,
    PacketType_State,
    PacketType_Notification,
};

enum Order
{
    Order_MoveTo,
    Order_Infiltrate,
    Order_Capture,
    Order_Stakeout,
    Order_Hack,
    Order_Intel
};

enum Notification
{
    Notification_AgentCaptured,
    Notification_AgentLost,
    Notification_IntelDetected,
    Notification_IntelCaptured,
    Notification_CrimeDetected,
    Notification_LineUsed,
    Notification_AgentSpotted,
    Notification_HouseDestroyed,
    Notification_Count,
};

struct InitializeGamePacket
{
    char        packetType;
    float       time;
    int         clientId;
    int         mapSeed;
    int         gridSpacing;
    int         xMapSize;
    int         yMapSize;
};

struct OrderPacket
{
    char        packetType;
    Order       order;
    int         agentId;

    union
    {
        int     targetStop;
    };
};

struct StatePacketHeader
{
    char        packetType;
    size_t      dataSize;
};

struct StatePacket
{
    StatePacketHeader header;
    char        data[1];
};

struct NotificationPacket
{
    char            packetType;
    Notification    notification;

    int             agentId;
    int             stop;
    int             line;
};

}

#endif
