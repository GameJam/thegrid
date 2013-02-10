#include "UI.h"
#include "OpenGL.h"
#include "Font.h"
#include "Texture.h"

#include <SDL.h>

static GLuint   g_texture           = 0;
static int      g_xMouse            = 0;
static int      g_yMouse            = 0;
static int      g_hotItem           = 0;
static int      g_activeItem        = 0;
static int      g_mouseButton[3]    = { 0, 0, 0 };
static char     g_charEvent         = 0;
static int      g_keyEvent          = 0;
static int      g_mousePressed[3]   = { 0, 0, 0 }; /* Mouse was pressed since the last frame */

void UI_CharacterEvent(char c)
{
    g_charEvent = c;
}

void UI_KeyEvent(int key)
{
    g_keyEvent = key;
}

void UI_Begin(GLuint texture, int xSize, int ySize)
{

    Uint8   buttons;
    int     i;

    g_texture = texture;

    SDL_Surface* surface;
    surface = SDL_GetVideoSurface();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0f, xSize, ySize, 0.0f);

    buttons = SDL_GetMouseState(&g_xMouse, &g_yMouse);

    for (i = 0; i < 3; ++i)
    {
        int prev = g_mouseButton[i];
        g_mouseButton[i] = buttons & SDL_BUTTON(i + 1);
        g_mousePressed[i] = !prev && g_mouseButton[i];
    }

    g_xMouse = g_xMouse * xSize / surface->w;
    g_yMouse = g_yMouse * ySize / surface->h;

    g_hotItem = 0;

}

void UI_End()
{
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    g_charEvent = 0;
    g_keyEvent  = 0;

}

int UI_GetIsMouseInside(int x, int y, int xSize, int ySize)
{
    return g_xMouse >= x && g_yMouse >= y && g_xMouse < x + xSize && g_yMouse < y + ySize;
}

void UI_DrawBox(int x, int y, int xSize, int ySize, int u, int v, int uSize, int vSize, int he, int ve)
{
    
    /*

    +---+-------+---+
    | 1 |   5   | 2 |
    +---+-------+---+
    |   |       |   |
    | 8 |   9   | 6 |
    |   |       |   |
    +---+-------+---+
    | 4 |   7   | 3 |
    +-----------+---+

    */

    const float xTexSize = 256.0;
    const float yTexSize = 256.0;

    float u1, v1, u2, v2;

    u1 = u / 256.0f;
    v1 = v / 256.0f;

    u2 = u1 + uSize / 256.0f;
    v2 = v1 + vSize / 256.0f;

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);

    /* Region 1 */

    glTexCoord2f(u / xTexSize, v / xTexSize);
    glVertex2i(x, y);

    glTexCoord2f((u + he) / xTexSize, v / xTexSize);
    glVertex2i(x + he, y);

    glTexCoord2f((u + he) / xTexSize, (v + ve) / xTexSize);
    glVertex2i(x + he, y + ve);

    glTexCoord2f(u / xTexSize, (v + ve) / xTexSize);
    glVertex2i(x, y + ve);

    /* Region 2 */

    glTexCoord2f((u + uSize - he) / xTexSize, v / xTexSize);
    glVertex2i(x + xSize - he, y);

    glTexCoord2f((u + uSize) / xTexSize, v / xTexSize);
    glVertex2i(x + xSize, y);

    glTexCoord2f((u + uSize) / xTexSize, (v + ve) / xTexSize);
    glVertex2i(x + xSize, y + ve);

    glTexCoord2f((u + uSize - he) / xTexSize, (v + ve) / xTexSize);
    glVertex2i(x + xSize - he, y + ve);

    /* Region 3 */

    glTexCoord2f((u + uSize - he) / xTexSize, (v + vSize - ve) / xTexSize);
    glVertex2i(x + xSize - he, y + ySize - ve);

    glTexCoord2f((u + uSize) / xTexSize, (v + vSize - ve) / xTexSize);
    glVertex2i(x + xSize, y + ySize - ve);

    glTexCoord2f((u + uSize) / xTexSize, (v + vSize) / xTexSize);
    glVertex2i(x + xSize, y + ySize);

    glTexCoord2f((u + uSize - he) / xTexSize, (v + vSize) / xTexSize);
    glVertex2i(x + xSize - he, y + ySize);

    /* Region 4 */

    glTexCoord2f(u / xTexSize, (v + vSize - ve) / xTexSize);
    glVertex2i(x, y + ySize - ve);

    glTexCoord2f((u + he) / xTexSize, (v + vSize - ve) / xTexSize);
    glVertex2i(x + he, y + ySize - ve);

    glTexCoord2f((u + he) / xTexSize, (v + vSize) / xTexSize);
    glVertex2i(x + he, y + ySize);

    glTexCoord2f(u / xTexSize, (v + vSize) / xTexSize);
    glVertex2i(x, y + ySize);

    /* Region 5 */

    glTexCoord2f((u + he) / xTexSize, v / xTexSize);
    glVertex2i(x + he, y);

    glTexCoord2f((u + uSize - he) / xTexSize, v / xTexSize);
    glVertex2i(x + xSize - he, y);

    glTexCoord2f((u + uSize - he) / xTexSize, (v + ve) / xTexSize);
    glVertex2i(x + xSize - he, y + ve);

    glTexCoord2f((u + he) / xTexSize, (v + ve) / xTexSize);
    glVertex2i(x + he, y + ve);
    
    /* Region 6 */

    glTexCoord2f((u + uSize - he) / xTexSize, (v + ve) / xTexSize);
    glVertex2i(x + xSize - he, y + ve);

    glTexCoord2f((u + uSize) / xTexSize, (v + ve) / xTexSize);
    glVertex2i(x + xSize, y + ve);

    glTexCoord2f((u + uSize) / xTexSize, (v + vSize - ve) / xTexSize);
    glVertex2i(x + xSize, y + ySize - ve);

    glTexCoord2f((u + uSize - he) / xTexSize, (v + vSize - ve) / xTexSize);
    glVertex2i(x + xSize - he, y + ySize - ve);

    /* Region 7 */

    glTexCoord2f((u + he) / xTexSize, (v + vSize - he) / xTexSize);
    glVertex2i(x + he, y + ySize - ve);

    glTexCoord2f((u + uSize - he) / xTexSize, (v + vSize - he) / xTexSize);
    glVertex2i(x + xSize - he, y + ySize - ve);

    glTexCoord2f((u + uSize - he) / xTexSize, (v + vSize) / xTexSize);
    glVertex2i(x + xSize - he, y + ySize);

    glTexCoord2f((u + he) / xTexSize, (v + vSize) / xTexSize);
    glVertex2i(x + he, y + ySize);

    /* Region 8 */

    glTexCoord2f(u / xTexSize, (v + ve) / xTexSize);
    glVertex2i(x, y + ve);

    glTexCoord2f((u + he) / xTexSize, (v + ve) / xTexSize);
    glVertex2i(x + he, y + ve);

    glTexCoord2f((u + he) / xTexSize, (v + vSize - ve) / xTexSize);
    glVertex2i(x + he, y + ySize - ve);

    glTexCoord2f(u / xTexSize, (v + vSize - ve) / xTexSize);
    glVertex2i(x, y + ySize - ve);

    /* Region 9 */

    glTexCoord2f((u + he) / xTexSize, (v + ve) / xTexSize);
    glVertex2i(x + he, y + ve);

    glTexCoord2f((u + uSize - he) / xTexSize, (v + ve) / xTexSize);
    glVertex2i(x + xSize - he, y + ve);

    glTexCoord2f((u + uSize - he) / xTexSize, (v + vSize - ve) / xTexSize);
    glVertex2i(x + xSize - he, y + ySize - ve);

    glTexCoord2f((u + he) / xTexSize, (v + vSize - ve) / xTexSize);
    glVertex2i(x + he, y + ySize - ve);

    glEnd();

}
void UI_HandleFocus(int id, int x,  int y, int xSize, int ySize)
{

    if (UI_GetIsMouseInside(x, y, xSize, ySize))
    {
        g_hotItem = id;
        if (g_mousePressed[0])
        {
            g_activeItem = id;
        }
    }

}

int UI_Button(int id, const Font& font, int x, int y, int xSize, int ySize, const char* label)
{

    int textWidth, textHeight;
    int textOffset;
    int v = 0;

    textWidth  = Font_GetTextWidth(font, label);
    textHeight = Font_GetTextHeight(font);

    UI_HandleFocus(id, x, y, xSize, ySize);

    if (g_hotItem == id)
    {
        if (g_activeItem == id)
        {
            v = 32;
        }
        else
        {
            v = 16;
        }
    }
    else
    {
        v = 0;
    }


    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, g_texture);

    UI_DrawBox(x, y, xSize, ySize, 0, v, 16, 16, 3, 3);

    Font_BeginDrawing(font);

    textOffset = g_activeItem == id ? 1 : 0;

    glColor3f(0.25f, 0.25f, 0.25f);
    Font_DrawText(label, x + (xSize - textWidth) / 2 + 1 + textOffset, y + (ySize - textHeight) / 2 + 1 + textOffset);

    glColor3f(1, 1, 1);
    Font_DrawText(label, x + (xSize - textWidth) / 2 + textOffset, y + (ySize - textHeight) / 2 + textOffset);
    Font_EndDrawing();

    if (!g_mouseButton[0] && g_activeItem == id)
    {
        g_activeItem = 0;
        if (g_hotItem == id)
        {
            return 1;
        }
     }

    return 0;

}

void UI_TextBox(int id, const Font& font, int x, int y, int xSize, int ySize, char* text, size_t maxLength)
{

    int textHeight;
    int cursorPos;
    size_t length;

    length = strlen(text);

    textHeight = Font_GetTextHeight(font);

    UI_HandleFocus(id, x, y, xSize, ySize);

    if (g_activeItem == id)
    {

        Uint8* keyState = SDL_GetKeyState(NULL);

        if (g_charEvent != 0 && length < maxLength)
        {
            text[length] = g_charEvent;
            text[length + 1] = 0;
            ++length;
        }
        else if (g_keyEvent == SDLK_BACKSPACE)
        {
            if (length > 0)
            {
                text[length - 1] = 0;
                --length;
            }
        }

    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, g_texture);

    UI_DrawBox(x, y, xSize, ySize, 0, 48, 16, 16, 3, 3);

    Font_BeginDrawing(font);

    glColor3f(1.0f, 1.0f, 1.0f);
    cursorPos = Font_DrawText(text, x + 5, y + (ySize - textHeight) / 2);

    if (g_activeItem == id)
    {
        if ((SDL_GetTicks() % 800) < 400)
        {
            Font_DrawText("_", cursorPos, y + (ySize - textHeight) / 2);
        }
    }

    Font_EndDrawing();

}

void UI_Label(const Font& font, int x, int y, int xSize, int ySize, const char* text)
{   

    glColor3f(1.0f, 1.0f, 1.0f);
    Font_BeginDrawing(font);

    Font_DrawText(text, x, y);

    Font_EndDrawing();

}
