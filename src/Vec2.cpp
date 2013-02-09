#include "Vec2.h"

void Vec2::Normalize()
{
    *this = *this / sqrtf(DotProduct(*this, *this));
}
