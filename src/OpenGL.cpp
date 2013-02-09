#include "OpenGL.h"

void glColor(unsigned long color)
{
    int b = (color) & 0xFF;
    int g = (color >> 8) & 0xFF;
    int r = (color >> 16) & 0xFF;
    int a = (color >> 24) & 0xFF;
    glColor4f( r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}