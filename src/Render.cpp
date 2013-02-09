#include "Render.h"
#include "Texture.h"

GLuint Render_CreateTexture(int xSize, int ySize, const void* buffer, int mipMap)
{

    GLuint textureId;
    glGenTextures(1, &textureId);

    glBindTexture(GL_TEXTURE_2D, textureId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    if (mipMap)
    {
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGBA, xSize, ySize, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
    }
    else
    {
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, xSize, ySize, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    }

    return textureId;

}

void Render_DrawSprite(const Texture& texture, int x, int y)
{

    glBindTexture( GL_TEXTURE_2D, texture.handle );
    glBegin(GL_QUADS);

    glTexCoord2f(0, 0);
    glVertex2i(x, y);

    glTexCoord2f(1, 0);
    glVertex2i(x + texture.xSize, y);

    glTexCoord2f(1, 1);
    glVertex2i(x + texture.xSize, y + texture.ySize);

    glTexCoord2f(0, 1);
    glVertex2i(x, y + texture.ySize);

    glEnd();

}
