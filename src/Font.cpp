#include "Font.h"
#include "Texture.h"

#include <stdio.h>
#include <assert.h>

/* The font we're currently drawing with */
static const Font* g_font = NULL;

int Font_Load(Font& font, const char* fileName)
{

    /* Font CSV files can be exporter with Codeheads Bitmap Font Generator */

    FILE* file = fopen(fileName, "rt");

    if (file == NULL)
    {
        return 0;
    }

    while (!feof(file))
    {

        char    buffer[256];
        char*   value;
        int     charIndex;
        int     matchLength;
        size_t  lineLength;

        fgets(buffer, sizeof(buffer), file);
        value = strchr(buffer, ',');

        lineLength = strlen(buffer);
        if (lineLength > 0 && buffer[lineLength - 1] == '\n')
        {
            buffer[lineLength - 1] = 0;
        }

        if (value != NULL)
        {
            
            value[0] = 0;
            ++value;

            if (sscanf(buffer, "Char %d %n", &charIndex, &matchLength) == 1)
            {
                if (charIndex < 256)
                {
                    if (strcmp(buffer + matchLength, "Base Width") == 0)
                    {
                        font.charWidth[charIndex] = atoi(value);
                    }
                }
            }
            if (strcmp(buffer, "Texture File") == 0)
            {
                Texture_Load(font.texture, value);
            }
            else if (strcmp(buffer, "Cell Width") == 0)
            {
                font.cellWidth = atoi(value);
            }
            else if (strcmp(buffer, "Cell Height") == 0)
            {
                font.cellHeight = atoi(value);
            }
            else if (strcmp(buffer, "Image Width") == 0)
            {
                font.imageWidth = atoi(value);
            }
            else if (strcmp(buffer, "Image Height") == 0)
            {
                font.imageHeight = atoi(value);
            }
            else if (strcmp(buffer, "Start Char") == 0)
            {
                font.startChar = atoi(value);
            }
            else if (strcmp(buffer, "Font Height") == 0)
            {
                font.fontHeight = atoi(value);
            }

        }

    }

    font.numRows = font.imageWidth  / font.cellWidth;
    font.numCols = font.imageHeight / font.cellHeight;

    fclose(file);
    file = NULL;

    return 1;

}

void Font_BeginDrawing(const Font& font)
{

    assert(g_font == NULL);
    g_font = &font;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, font.texture.handle);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void Font_EndDrawing()
{
    assert(g_font != NULL);
    g_font = NULL;
}

int Font_DrawText(const char* text, int x, int y)
{

    assert(g_font != NULL);

    glBegin(GL_QUADS);

    int xStart = x;

    while (text[0] != 0)
    {

        while (text[0] == '\n')
        {
            y += g_font->fontHeight;
            x = xStart;
            ++text;
        }
    
        int     charIndex;
        int     row, col;
        int     x1, y1, x2, y2;
        float   u1, v1, u2, v2;
        int     width;

        charIndex = (unsigned char)text[0];

        row = (charIndex - g_font->startChar) / g_font->numCols;
        col = (charIndex - g_font->startChar) % g_font->numCols;

        width = g_font->charWidth[charIndex];

        x1 = x;
        y1 = y;
        x2 = x + width;
        y2 = y + g_font->cellHeight;

        u1 = (float)(col) / g_font->numCols;
        v1 = (float)(row) / g_font->numRows;
        u2 = u1 + (float)(width) / g_font->imageWidth;
        v2 = v1 + 1.0f / g_font->numRows;

        glTexCoord2f(u1, v1);
        glVertex2i(x1, y1);

        glTexCoord2f(u2, v1);
        glVertex2i(x2, y1);

        glTexCoord2f(u2, v2);
        glVertex2i(x2, y2);

        glTexCoord2f(u1, v2);
        glVertex2i(x1, y2);

        x += width;

        ++text;

    }

    glEnd();
    return x;

}

int Font_GetTextWidth(const Font& font, const char* text)
{

    int width = 0;
    
    while (text[0] != 0)
    {
        int charIndex = (unsigned char)text[0];
        width += font.charWidth[charIndex];
        ++text;
    }

    return width;

}

int Font_GetTextHeight(const Font& font)
{
    return font.fontHeight;
}
