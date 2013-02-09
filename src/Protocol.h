#ifndef GAME_PROTOCOL_H
#define GAME_PROTOCOL_H

namespace Protocol
{

enum PacketType
{
    PacketType_Order
};

struct OrderPacket
{
    char packetType;
    int agentId;
    int x, y;
};

}

#endif
