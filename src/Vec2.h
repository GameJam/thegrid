#ifndef GAME_VEC2_H
#define GAME_VEC2_H

#include <math.h>

class Vec2
{

public:

    Vec2() { }
    Vec2(int _x, int _y) : x((float)_x), y((float)_y) { }
    Vec2(float _x, float _y) : x(_x), y(_y) { }

    void Normalize();

public:

    float x;
    float y;

};

inline Vec2 operator+(const Vec2& u, const Vec2& v) { return Vec2(u.x + v.x, u.y + v.y); }
inline Vec2 operator-(const Vec2& u, const Vec2& v) { return Vec2(u.x - v.x, u.y - v.y); }
inline Vec2 operator*(float s, const Vec2& v) { return Vec2(s * v.x, s * v.y); }
inline Vec2 operator/(const Vec2& v, float s) { return Vec2(v.x / s, v.y / s); }

inline float DotProduct(const Vec2& u, const Vec2& v) { return u.x * v.x + u.y * v.y; }
inline Vec2 Lerp(const Vec2& a, const Vec2& b, float t) { return a + t*(b - a); }

#endif