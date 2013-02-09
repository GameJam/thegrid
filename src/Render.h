#ifndef GAME_RENDER_H
#define GAME_RENDER_H

#include "OpenGL.h"

/**
 * Creates a new texture from the RGBA pixel data.
 */
GLuint Render_CreateTexture(int xSize, int ySize, const void* buffer, int mipMap);

#endif