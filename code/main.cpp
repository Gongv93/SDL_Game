#include "SDL.h" 
#include "SDL_events.h"
#include "SDL_log.h"
#include "SDL_timer.h"
#include "SDL_rect.h"

#include <stdint.h>

#include <windows.h>

#define local_persist   static
#define global_variable static
#define internal        static

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32   bool32;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

internal void
Render(SDL_Surface *Surface) 
{
    // 32 bits per pixel, 8 bits per color
    // xx RR GG BB xx BB GG RR

    int Width  = Surface->w;
    int Height = Surface->h;

    uint8 *Row = (uint8 *)Surface->pixels;

    if (!SDL_LockSurface(Surface))
    {
        for(int y = 0; y < Height; ++y) {
            uint32 *Pixel = (uint32 *) Row;
            for(int x = 0; x < Width; ++x) {
                uint8 Blue = x;
                uint8 Green = y;

                *Pixel++ = ((Green << 8) | Blue);
            }
            Row += Surface->pitch;
        }
    }
    else {
        // TODO(Vincent): Logging
    }
    SDL_UnlockSurface(Surface);

}

internal void
DisplayBufferInWindow(SDL_Window *Window)
{
    SDL_UpdateWindowSurface(Window);
}

int 
main(int argc, char* args[]) 
{ 
    bool Running = true;
	if(SDL_Init(SDL_INIT_VIDEO)) {
		//SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Init Failed");
	}

    SDL_Window *Window = NULL;
	Window = SDL_CreateWindow("Window",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              1280,
                              780,
                              0);

    SDL_Surface *WindowSurface = NULL;
    WindowSurface = SDL_GetWindowSurface(Window);

    SDL_Event Event;
    while(Running) {
        if(SDL_WaitEvent(&Event)) {
            switch(Event.type) {
                case SDL_QUIT :
                {
                    Running = false;
                } break;
            }
        }
        else {
            // TODO(Vincent): Logging
        }

        Render(WindowSurface);
        DisplayBufferInWindow(Window);
    }

	return 0;
}
