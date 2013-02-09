#ifndef GAME_TEXTURE_H
#define GAME_TEXTURE_H

#include "Render.h"

struct Texture
{
    int     xSize;
    int     ySize;
    GLuint  handle;
};


/**
 * Loads a texture from disk.
 */
bool Texture_Load(Texture& texture, const char* fileName);

#endif