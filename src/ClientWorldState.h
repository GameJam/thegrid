#ifndef GAME_CLIENT_WORLD_STATE_H
#define GAME_CLIENT_WORLD_STATE_H

#include <vector>

class ClientWorldState
{

public:

    ClientWorldState();

    size_t GetSerializedSize();
    void Serialize(void* buffer);
    void Deserialize(const void* buffer);

    int m_id;
    struct Test { int x; int y; };
    std::vector<Test> m_test;

};

#endif
