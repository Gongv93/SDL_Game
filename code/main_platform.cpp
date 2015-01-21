#include "SDL2/SDL.h" 
#include "SDL2/SDL_events.h"
#include "SDL2/SDL_timer.h"

//#include "SDL2/SDL_mixer.h"

#include <stdint.h>
#include <stdio.h>
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

typedef float  real32;
typedef double real64;


global_variable uint64 GlobalPerformanceFrequency;

struct game_state
{
    int BlueOffset;
    int GreenOffset;
};

struct controller_state
{
    bool32 MoveUp;
    bool32 MoveDown;
    bool32 MoveLeft;
    bool32 MoveRight;
};

internal void
Render(SDL_Surface *Surface, int BlueOffset, int GreenOffset) 
{
    // 32 bits per pixel, 8 bits per color
    // xx RR GG BB

    int Width  = Surface->w;
    int Height = Surface->h;

    uint8 *Row = (uint8 *)Surface->pixels;

    if (!SDL_LockSurface(Surface))
    {
        for(int y = 0; y < Height; ++y) {
            uint32 *Pixel = (uint32 *) Row;
            for(int x = 0; x < Width; ++x) {
                uint8 Blue = (uint8)(x + BlueOffset);
                uint8 Green = (uint8)(y + GreenOffset);

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

internal uint64 
PlatformGetTime(void)
{
    return SDL_GetPerformanceCounter();
}

internal real32
PlatformGetSecondsElapsed(uint64 Start, uint64 End)
{
    real32 Result = ((real32)(End - Start)) /
                     (real32)(GlobalPerformanceFrequency);
    return Result;
}

internal void
PlatformDisplayBufferInWindow(SDL_Window *Window)
{
    SDL_UpdateWindowSurface(Window);
}

int 
main(int argc, char* args[]) 
{ 

    bool Running = true;
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER)) {
        // TODO(Vincent): Logging
	}

    // Get a window
    SDL_Window *Window = NULL;
	Window = SDL_CreateWindow("Window",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              1280,
                              780,
                              0);

    // Get a surface to draw on
    SDL_Surface *WindowSurface = NULL;
    WindowSurface = SDL_GetWindowSurface(Window);

    // Game var inits 
    game_state GameState = {};
    controller_state Contoller = {};

    // Init timer for enforcing FPS
    int MonitorRefreshHz = 60;
    int GameUpdateHz = MonitorRefreshHz/2;
    real32 TargetTimePerFrame = 1.0f / (real32)GameUpdateHz;

    GlobalPerformanceFrequency = SDL_GetPerformanceFrequency();
    uint64 StartCounter = SDL_GetPerformanceCounter();

    SDL_Event Event;
    while(Running) {
        if(SDL_PollEvent(&Event)) {
            switch(Event.type) {
                case SDL_QUIT :
                {
                    Running = false;
                } break;

                case SDL_KEYUP :
                case SDL_KEYDOWN :
                {
                    switch(Event.key.keysym.sym) {
                        case SDLK_w :
                        {
                            Contoller.MoveUp = Event.key.state;
                        } break;
                        case SDLK_s :
                        {
                            Contoller.MoveDown = Event.key.state;
                        } break;
                        case SDLK_d :
                        {
                            Contoller.MoveRight = Event.key.state;
                        } break;
                        case SDLK_a :
                        {
                            Contoller.MoveLeft = Event.key.state;
                        } break;
                        default:
                        {

                        } break;
                    }
                } break;
            }
        }
        else {
            // TODO(Vincent): Logging
        }

        // Game state update
        if(Contoller.MoveUp)
            GameState.GreenOffset -= 5;
        if(Contoller.MoveDown)
            GameState.GreenOffset += 5;
        if(Contoller.MoveRight)
            GameState.BlueOffset += 5;
        if(Contoller.MoveLeft)
            GameState.BlueOffset -= 5;

        uint64 EndCounter = PlatformGetTime();

        real32 TimePerFrame = PlatformGetSecondsElapsed(StartCounter, EndCounter);
        if(TimePerFrame < TargetTimePerFrame) {
            DWORD SleepTime = (DWORD)(1000.0f * (TargetTimePerFrame -
                                                 TimePerFrame));
            SDL_Delay(SleepTime);

            while(TimePerFrame < TargetTimePerFrame) {
                TimePerFrame = PlatformGetSecondsElapsed(StartCounter,
                                                         PlatformGetTime());
            }
        }
        else {
            // TODO(Vincent): MISSED FRAME
            OutputDebugStringA("MISSED FRAME\n");
        }

        real32 MSPerFrame = 1000.0f*PlatformGetSecondsElapsed(StartCounter, EndCounter);

        char FPSBuffer[256];
        _snprintf_s(FPSBuffer, sizeof(FPSBuffer),
                    "%.02fms/f\n", MSPerFrame);
        OutputDebugStringA(FPSBuffer);

        StartCounter = EndCounter;

        // Show frame on screen after state update
        Render(WindowSurface, GameState.BlueOffset, GameState.GreenOffset);
        PlatformDisplayBufferInWindow(Window);
    }

    SDL_Quit();

	return 0;
}
