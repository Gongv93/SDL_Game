#include "SDL2/SDL.h" 
#include "SDL2/SDL_events.h"
#include "SDL2/SDL_timer.h"
#include "SDL2/SDL_audio.h"
#include "SDL2/SDL_render.h"

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

struct SDL_offscreen_buffer
{
    SDL_Renderer *Renderer;
    SDL_Texture *Texture;
    void *Pixels;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

internal void
Render(SDL_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset) 
{
    // 32 bits per pixel, 8 bits per color
    // xx RR GG BB

    int Width  = Buffer->Width;
    int Height = Buffer->Height;

    uint8 *Row = (uint8 *)Buffer->Pixels;
    for(int y = 0; y < Height; ++y) {
        uint32 *Pixel = (uint32 *) Row;
        for(int x = 0; x < Width; ++x) {
            uint8 Blue = (uint8)(x + BlueOffset);
            uint8 Green = (uint8)(y + GreenOffset);

            *Pixel++ = ((Green << 8) | Blue);
        }
        Row += Buffer->Pitch;
    }

}

internal void
SDLDisplayBufferInWindow(SDL_offscreen_buffer *Buffer)
{
    //SDL_UpdateWindowSurface(Window);
    if(!SDL_LockTexture(Buffer->Texture, NULL, &Buffer->Pixels, &Buffer->Pitch)) {
        SDL_UnlockTexture(Buffer->Texture);
    }
    else {
        // TODO(Vincent): Logging
    }

    SDL_RenderCopy(Buffer->Renderer,
                   Buffer->Texture,
                   NULL, NULL);
    SDL_RenderPresent(Buffer->Renderer);
}

internal void
SDLResizeTexture(SDL_offscreen_buffer *Buffer, int Width, int Height)
{
    // TODO(Vincent): Mayby use virtualalloc for windows
    // and mmap for linux builds
    if(Buffer->Pixels != NULL) {
        free(Buffer->Pixels);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4;
    Buffer->Pitch = Buffer->Width * Buffer->BytesPerPixel; 

    Buffer->Texture = SDL_CreateTexture(Buffer->Renderer, 
                                        SDL_PIXELFORMAT_ARGB8888,
                                        SDL_TEXTUREACCESS_STREAMING,
                                        Width, Height);

    int TextureMemorySize = (Buffer->Width * Buffer->Height) * Buffer->BytesPerPixel;

    Buffer->Pixels = malloc(TextureMemorySize);
}

internal uint64 
SDLGetTime(void)
{
    return SDL_GetPerformanceCounter();
}

internal real32
SDLGetSecondsElapsed(uint64 Start, uint64 End)
{
    real32 Result = ((real32)(End - Start)) /
                     (real32)(GlobalPerformanceFrequency);
    return Result;
}

int 
main(int argc, char* args[]) 
{ 

    bool Running = true;
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER)) {
        // TODO(Vincent): Logging
	}

    // Get a window
    int Width;
    int Height;
    SDL_Window *Window = SDL_CreateWindow("Window",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          1280,
                                          780,
                                          0);
    SDL_GetWindowSize(Window, &Width, &Height);

    // Set up video buffer
    SDL_offscreen_buffer offscreen_buffer = {};
    offscreen_buffer.Renderer = SDL_CreateRenderer(Window, -1, 0);

    SDLResizeTexture(&offscreen_buffer, Width, Height);

    // Set up audio
    //SDL_AudioDeviceID AudioDeviceID = NULL;
    //uint32 AudioSize = SDL_GetQueuedAudioSize(AudioDeviceID); 


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
        while(SDL_PollEvent(&Event)) {
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

        // Update the game for the next frame
        if(Contoller.MoveUp)
            GameState.GreenOffset -= 5;
        if(Contoller.MoveDown)
            GameState.GreenOffset += 5;
        if(Contoller.MoveRight)
            GameState.BlueOffset += 5;
        if(Contoller.MoveLeft)
            GameState.BlueOffset -= 5;

        Render(&offscreen_buffer, GameState.BlueOffset, GameState.GreenOffset);



        // Sets a fixed frame rate by sleeping if the update
        // finished too early 
        uint64 EndCounter = SDLGetTime();

        real32 CurrentTimePerFrame = SDLGetSecondsElapsed(StartCounter, EndCounter);
        if(CurrentTimePerFrame < TargetTimePerFrame) {
            DWORD SleepTime = (DWORD)(1000.0f * (TargetTimePerFrame -
                                                 CurrentTimePerFrame));
            if(SleepTime > 0) {
                SDL_Delay(SleepTime);
            }

            while(CurrentTimePerFrame < TargetTimePerFrame) {
                CurrentTimePerFrame = SDLGetSecondsElapsed(StartCounter,
                                                         SDLGetTime());
            }
        }
        else {
            // TODO(Vincent): Do something about a missed frame
            OutputDebugStringA("MISSED FRAME\n");
        }

        // Show frame on screen after state update
        SDLDisplayBufferInWindow(&offscreen_buffer);

        real32 MSPerFrame = 1000.0f*SDLGetSecondsElapsed(StartCounter, EndCounter);

        char FPSBuffer[256];
        _snprintf_s(FPSBuffer, sizeof(FPSBuffer),
                    "%.02fms/f\n", MSPerFrame);
        OutputDebugStringA(FPSBuffer);

        StartCounter = EndCounter;
    }

    SDL_Quit();

	return 0;
}
