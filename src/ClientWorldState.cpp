#include "ClientWorldState.h"

ClientWorldState::ClientWorldState()
{
    m_id = -1;
}

size_t ClientWorldState::GetSerializedSize()
{
    return sizeof(Test) * m_test.size() + 2*sizeof(int);
}

void ClientWorldState::Serialize(void* buffer)
{
    int* intBuffer = static_cast<int*>(buffer);

    *intBuffer = m_id;
    ++intBuffer;

    *intBuffer = static_cast<int>(m_test.size());
    ++intBuffer;

    for (size_t i = 0; i < m_test.size(); ++i)
    {
        *intBuffer = m_test[i].x;
        ++intBuffer;

        *intBuffer = m_test[i].y;
        ++intBuffer;
    }
}

void ClientWorldState::Deserialize(const void* buffer)
{
    const int* intBuffer = static_cast<const int*>(buffer);

    m_id = *intBuffer;
    ++intBuffer;

    size_t size = *intBuffer;
    ++intBuffer;

    m_test.resize(size);

    for (size_t i = 0; i < size; ++i)
    {
        m_test[i].x = *intBuffer;
        ++intBuffer;

        m_test[i].y = *intBuffer;
        ++intBuffer;
    }
}
