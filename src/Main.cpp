#include "Render.h"
#include "Texture.h"
#include "ClientGame.h"
#include "Server.h"
#include "Log.h"

#include <SDL.h>
#include <SDL_syswm.h>
#include <bass.h>

#include <string>
#include <hash_map>

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

typedef stdext::hash_map<std::string, std::string> Arguments;

bool ParseArguments(int argc, char* argv[], Arguments& arguments)
{

    arguments.clear();
    for (int i = 1; i + 1 < argc; i += 2)
    {
        const char* key = argv[i];
        const char* value = argv[i + 1];
        
        if (key[0] != '-')
        {
            LogError("Expected key (prefixed with '-'): '%s'", key);
            return false;
        }

        if (value[0] == '-')
        {
            LogError("Expected value, got key instead: '%s'", value);
            return false;
        }

        arguments[key + 1] = value;
    }

    return true;

}

bool HasArgument(const Arguments& arguments, const char* key)
{
    return arguments.find(key) != arguments.end();
}

const char* GetArgument(const Arguments& arguments, const char* key)
{

    Arguments::const_iterator iter = arguments.find(key);
    if (iter == arguments.end())
    {
        return NULL;
    }

    return iter->second.c_str();

}

int main(int argc, char* argv[])
{

    const int xSize = 1280;
    const int ySize = 800;

    Log::Initialize(Log::Severity_Debug);
    LogMessage("Initializing the grid...");

    Arguments arguments;
    if (!ParseArguments(argc, argv, arguments))
    {
        exit(EXIT_FAILURE);
    }

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

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    SDL_EnableUNICODE(1);  

    SDL_Surface* screen = SDL_SetVideoMode(xSize, ySize, 32, SDL_OPENGL /*| SDL_FULLSCREEN*/);

    if ( screen == NULL )
    {
        fprintf(stderr, "Error: Unable to set %dx%d video: %s\n", xSize, ySize, SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_SysWMinfo sysInfo;
    SDL_VERSION(&sysInfo.version);
  
    if (SDL_GetWMInfo(&sysInfo) <= 0)
    {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    BASS_Init(-1, 44100, 0, sysInfo.window, NULL);

    const char* hostName = "127.0.0.1";
    Server* server = NULL;

    if (HasArgument(arguments, "connect"))
    {
        hostName = GetArgument(arguments, "connect");
    }
    else
    {
        server = new Server();
    }

    ClientGame* game = new ClientGame(xSize, ySize);    

    game->LoadResources();
    game->Connect(hostName, 12345);

    while (ProcessEvents(*game))
    {
        game->Update();
        game->Render();
        SDL_GL_SwapBuffers();
        
        if (server)
        {
            server->Update();
        }
    }

    delete game;
    game = NULL;

    delete server;
    server = NULL;

    Host::Shutdown();
    Log::Shutdown();

    BASS_Free();

    return EXIT_SUCCESS;

}
