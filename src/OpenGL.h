#ifndef GAME_OPENGL_H
#define GAME_OPENGL_H

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

#include "Vec2.h"

void glColor(unsigned long color);
inline void glVertex(const Vec2& point) { glVertex2f(point.x, point.y); }

#endif