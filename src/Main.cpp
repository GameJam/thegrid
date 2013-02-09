#include "Render.h"
#include "Texture.h"
#include "ClientGame.h"
#include "Server.h"
#include "Client.h"
#include "Log.h"

#include <SDL.h>


bool ProcessEvents(ClientGame& game)
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

        case SDL_MOUSEBUTTONDOWN:
            game.OnMouseDown(event.button.x, event.button.y, event.button.button);
            break;

        case SDL_MOUSEBUTTONUP:
            game.OnMouseUp(event.button.x, event.button.y, event.button.button);
            break;

        case SDL_MOUSEMOTION:
            game.OnMouseMove(event.motion.x, event.motion.y);
            break;


        }
    }

    return true;

}

int main(int argc, char* argv[])
{

    const int xSize = 1280;
    const int ySize = 800;

    Log::Initialize(Log::Severity_Debug);
    LogMessage("Initializing the grid...");

    Host::Initialize();

    if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0 )
    {
        exit(EXIT_FAILURE);
    }

    atexit(SDL_Quit);

    SDL_WM_SetCaption("The Grid", NULL);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

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

    ClientGame game(xSize, ySize);
    
    Server server;
    Client client;
    client.Connect("127.0.0.1", 12345);

    game.LoadResources();

    while (ProcessEvents(game))
    {
        game.Render();
        SDL_GL_SwapBuffers();
        server.Update();
        client.Update();
    }

    Host::Shutdown();
    Log::Shutdown();

    return EXIT_SUCCESS;

}
