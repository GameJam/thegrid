#ifndef GAME_PROTOCOL_H
#define GAME_PROTOCOL_H

namespace Protocol
{

enum PacketType
{
    PacketType_Order,
    PacketType_State
};

struct OrderPacket
{
    char packetType;
    int agentId;
    int x, y;
};

struct StatePacket
{
    char packetType;
    char data[1];
};

}

#endif
