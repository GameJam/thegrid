#ifndef GAME_PROTOCOL_H
#define GAME_PROTOCOL_H

namespace Protocol
{

enum PacketType
{
    PacketType_InitializeGame,
    PacketType_Order,
    PacketType_State
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
    char data[1];
};

}

#endif
