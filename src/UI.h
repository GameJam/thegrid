#ifndef GAME_UI_H
#define GAME_UI_H

#include "OpenGL.h"

struct Font;

#define UI_ID (__LINE__)

/**
 * 
 */
void UI_Begin(GLuint texture, int xSize, int ySize);

/**
 *
 */
void UI_End();

/**
 * Should be called when the user types a character.
 */
void UI_CharacterEvent(char c);

/**
 * Should be called when the user presses a key.
 */
void UI_KeyEvent(int key);

/**
 * Creates a button in the UI. The method returns true if the button was clicked.
 */
int UI_Button(int id, const Font& font, int x, int y, int xSize, int ySize, const char* label);

/**
 * Creates a button in the UI. The method returns true if the button was clicked.
 */
void UI_TextBox(int id, const Font& font, int x, int y, int xSize, int ySize, char* text, size_t maxLength);

/**
 * Creates a static text label.
 */
void UI_Label(Font& font, int x, int y, int xSize, int ySize, const char* text);

#endif