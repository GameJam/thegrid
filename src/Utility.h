#ifndef GAME_UTILITY_H
#define GAME_UTILITY_H

template <class T>
T Min(const T& x, const T& y)
{
    return x < y ? x : y;
}

template <class T>
T Max(const T& x, const T& y)
{
    return x > y ? x : y;
}

template <class T>
T Clamp(const T& x, const T& minValue, const T& maxValue)
{
    return Max(minValue, Min(maxValue, x));
}

template <class T>
void Swap(T& x, T& y)
{
    T temp = x;
    x = y;
    y = temp;
}

#endif