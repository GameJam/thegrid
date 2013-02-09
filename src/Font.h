#ifndef GAME_FONT_H
#define GAME_FONT_H

#include "Texture.h"

struct Font
{
    Texture texture;
    int     charWidth[256];
    int     startChar;
    int     fontHeight;
    int     cellWidth;
    int     cellHeight;
    int     imageWidth;
    int     imageHeight;
    int     numRows;
    int     numCols;
};

/**
 * Loads a font file.
 */
int Font_Load(Font& font, const char* fileName);

/**
 * Begins drawing with the specified font.
 */
void Font_BeginDrawing(const Font& font);

/**
 * Finishes drawing with a font.
 */
void Font_EndDrawing();

/**
 * Draws text on the screen at the specified location.
 */
int Font_DrawText(const char* text, int x, int y);

/**
 * Returns the width (in pixels) of the text when rendered.
 */
int Font_GetTextWidth(const Font& font, const char* text);

/**
 * Returns the height (in pixels) of the text when rendered.
 */
int Font_GetTextHeight(const Font& font);

#endif