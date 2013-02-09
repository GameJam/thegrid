#ifndef GAME_VEC2_H
#define GAME_VEC2_H

class Vec2
{

public:

    Vec2() { }
    Vec2(float _x, float _y) : x(_x), y(_y) { }

public:

    float x;
    float y;

};

inline Vec2 operator+(const Vec2& u, const Vec2& v) { return Vec2(u.x + v.x, u.y + v.y); }
inline Vec2 operator-(const Vec2& u, const Vec2& v) { return Vec2(u.x - v.x, u.y - v.y); }
inline Vec2 operator*(float s, const Vec2& v) { return Vec2(s * v.x, s * v.y); }

#endif