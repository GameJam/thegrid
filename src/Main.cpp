#include "Render.h"
#include "Texture.h"

#include <SDL.h>

const int xSize = 1280;
const int ySize = 800;

Texture mapTexture;

bool ProcessEvents()
{

    SDL_Event event;

    while ( SDL_PollEvent(&event) )
    {
        switch (event.type)
        {

        case SDL_KEYDOWN:
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    return false;
                }
                
                char c = event.key.keysym.unicode & 0x7f;
                if (isgraph(c) || isspace(c) )
                {
                    // character c
                }
                else
                {
                    // key event.key.keysym.sym
                }
            }
            break;

        case SDL_QUIT:
            return false;

        }
    }

    return true;

}

void Render()
{

    Render_Begin(xSize, ySize);

    Render_DrawSprite(mapTexture, 0, 0);

    Render_End();

    SDL_GL_SwapBuffers();

}

void LoadResources()
{
    Texture_Load(mapTexture, "assets/map_subway.jpg");
}

int main(int argc, char* argv[])
{

    if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0 )
    {
        exit(EXIT_FAILURE);
    }

    atexit(SDL_Quit);

    SDL_WM_SetCaption("The Grid", NULL);

    /* Initialize the video. */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

    /* Disable vsync. */
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 0); 

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    SDL_EnableUNICODE(1);  

    SDL_Surface* screen = SDL_SetVideoMode(xSize, ySize, 32, SDL_OPENGL);

    if ( screen == NULL )
    {
        fprintf(stderr, "Error: Unable to set %dx%d video: %s\n", xSize, ySize, SDL_GetError());
        exit(EXIT_FAILURE);
    }

    LoadResources();

    while (ProcessEvents())
    {
        Render();
    }

    return EXIT_SUCCESS;

}
