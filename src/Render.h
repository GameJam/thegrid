#ifndef GAME_RENDER_H
#define GAME_RENDER_H

#include "OpenGL.h"

struct Texture;

void Render_Begin(int x, int y, int xSize, int ySize);
void Render_End();

/**
 * Creates a new texture from the RGBA pixel data.
 */
GLuint Render_CreateTexture(int xSize, int ySize, const void* buffer, int mipMap);

void Render_DrawSprite(const Texture& texture, int x, int y);

#endif