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

struct InitializeGamePacket
{
    char packetType;
    int mapSeed;
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
