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

#include "game.h"
#include "game.cpp"

#include "main_platform.h"

global_variable bool GlobalRunning;
global_variable uint64 GlobalPerformanceFrequency;
global_variable SDL_offscreen_buffer offscreen_buffer;

//
// Graphics functions
//

internal void
SDLDisplayBufferInWindow(SDL_offscreen_buffer *Buffer)
{
    //SDL_UpdateWindowSurface(Window);
    if(!SDL_UpdateTexture(Buffer->Texture, NULL, Buffer->Pixels, Buffer->Pitch)) {
        SDL_RenderCopy(Buffer->Renderer,
                   Buffer->Texture,
                   NULL, NULL);
        SDL_RenderPresent(Buffer->Renderer);
    }
    else {
        // TODO(Vincent): Logging
    }
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

//
// Timer fucntions
//

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

//
// Controller functions
//

internal real32
SDLProcessStickValue(int16 Value, int16 DeadZone)
{
    real32 Result = 0;
    if(Value < -DeadZone) {
        Result = (real32)Value / 32768.0f;
    }
    else if(Value > DeadZone) {
        Result = (real32)Value / 32768.0f;
    }

    return Result;
}

internal void
SDLProcessControllerInput(SDL_GameController *GameController, game_controller_state *NewController)
{
    if(SDL_GameControllerGetAttached(GameController)){
                NewController->IsConnected = true;

                int16 LeftStickX = SDL_GameControllerGetAxis(GameController, 
                                                             SDL_CONTROLLER_AXIS_LEFTX);
                int16 LeftStickY = SDL_GameControllerGetAxis(GameController, 
                                                           SDL_CONTROLLER_AXIS_LEFTY);


                NewController->StickAverageX = SDLProcessStickValue(LeftStickX, 
                                                                    LEFT_THUMB_DEADZONE);
                NewController->StickAverageY = SDLProcessStickValue(LeftStickY, 
                                                                    LEFT_THUMB_DEADZONE);

                if((NewController->StickAverageX != 0.0f) ||
                   (NewController->StickAverageY != 0.0f))
                {
                    NewController->IsAnalog = true;
                }

                if(SDL_GameControllerGetButton(GameController, 
                                               SDL_CONTROLLER_BUTTON_DPAD_UP))
                {
                    NewController->StickAverageY = 1.0f;
                    NewController->IsAnalog = false;
                }


                if(SDL_GameControllerGetButton(GameController, 
                                               SDL_CONTROLLER_BUTTON_DPAD_DOWN))
                {
                    NewController->StickAverageY = -1.0f;
                    NewController->IsAnalog = false;
                }


                if(SDL_GameControllerGetButton(GameController, 
                                               SDL_CONTROLLER_BUTTON_DPAD_LEFT))
                {
                    NewController->StickAverageX = -1.0f;
                    NewController->IsAnalog = false;
                }


                if(SDL_GameControllerGetButton(GameController, 
                                               SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
                {
                    NewController->StickAverageX = 1.0f;
                    NewController->IsAnalog = false;
                }

                real32 threshold = 0.5;
                NewController->MoveUp    = (NewController->StickAverageY > threshold);
                NewController->MoveDown  = (NewController->StickAverageY < -threshold);
                NewController->MoveLeft  = (NewController->StickAverageX < -threshold);
                NewController->MoveRight = (NewController->StickAverageX > threshold);

                NewController->ActionUp = 
                    SDL_GameControllerGetButton(GameController,
                                                SDL_CONTROLLER_BUTTON_Y);

                NewController->ActionDown = 
                    SDL_GameControllerGetButton(GameController,
                                                SDL_CONTROLLER_BUTTON_A);

                NewController->ActionLeft = 
                    SDL_GameControllerGetButton(GameController,
                                                SDL_CONTROLLER_BUTTON_X);

                NewController->ActionRight = 
                    SDL_GameControllerGetButton(GameController,
                                                SDL_CONTROLLER_BUTTON_B);
            }
            else {
                NewController->IsConnected = false;
            }
}

//
// Event functions
//

int 
SDLEventFilter(void* userdata, SDL_Event *Event)
{
    // NOTE(Vicnent): Really back hack to redraw the screen while resize
    // TODO(Vincent): Find a better way!!
    if(Event->window.event == SDL_WINDOWEVENT_RESIZED){
        SDLDisplayBufferInWindow(&offscreen_buffer);
    }
    return 1;
}

internal void
SDLEventHandler(game_controller_state *KeyboardController)
{
    SDL_Event Event;
    while(SDL_PollEvent(&Event)) {
        switch(Event.type) {
            case SDL_QUIT :
            {
                GlobalRunning = false;
            } break;

            case SDL_KEYUP :
            case SDL_KEYDOWN :
            {
                switch(Event.key.keysym.sym) {
                    case SDLK_w :
                    {            
                        KeyboardController->MoveUp = Event.key.state;
                    } break;
                    case SDLK_s :
                    {
                        KeyboardController->MoveDown = Event.key.state;
                    } break;
                    case SDLK_d :
                    {
                        
                        KeyboardController->MoveRight = Event.key.state;
                    } break;
                    case SDLK_a :
                    {
                        
                        KeyboardController->MoveLeft = Event.key.state;
                    } break;
                    case SDLK_q :
                    {
                        
                        KeyboardController->LeftShoulder = Event.key.state;
                    } break;
                    case SDLK_e :
                    {
                        
                        KeyboardController->RightShoulder = Event.key.state;
                    } break;
                    case SDLK_UP :
                    {
                        
                        KeyboardController->ActionUp = Event.key.state;
                    } break;
                    case SDLK_DOWN :
                    {
                        
                        KeyboardController->ActionDown = Event.key.state;
                    } break;
                    case SDLK_RIGHT :
                    {
                        
                        KeyboardController->ActionRight = Event.key.state;
                    } break;
                    case SDLK_LEFT :
                    {
                        
                        KeyboardController->ActionLeft = Event.key.state;
                    } break;
                    case SDLK_ESCAPE :
                    {
                        GlobalRunning = false;
                    } break;
                    default:
                    {

                    } break;
                }
            } break;
        }
    }
}


int 
main(int argc, char* args[]) 
{ 
    GlobalRunning = true;
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER|SDL_INIT_GAMECONTROLLER)) {
        // TODO(Vincent): Logging
	}

    // Get a window
    SDL_Window *Window = SDL_CreateWindow("Window",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          1280,
                                          780,
                                          SDL_WINDOW_RESIZABLE);


    // This is for our bad hack
    SDL_SetEventFilter((SDL_EventFilter)&SDLEventFilter, NULL);

    // Set up video buffer
    //SDL_GetWindowSize(Window, &offscreen_buffer.Width, &offscreen_buffer.Height);
    offscreen_buffer.Renderer = SDL_CreateRenderer(Window, -1, 0);

    SDLResizeTexture(&offscreen_buffer, 1280, 780);

    // Set up audio
    //SDL_AudioDeviceID AudioDeviceID = NULL;
    //uint32 AudioSize = SDL_GetQueuedAudioSize(AudioDeviceID); 

    // Set up controller
    // First controller will be keyboard the other 
    // four will be the controller
    int MaxJoysticks = SDL_NumJoysticks();
    int ControllerCount = 0;
    SDL_GameController *GameController[4];
    for(int JoystickIndex = 0; JoystickIndex < MaxJoysticks; ++JoystickIndex) {
        if(SDL_IsGameController(JoystickIndex) == SDL_TRUE) {
            GameController[ControllerCount++] = SDL_GameControllerOpen(JoystickIndex);
        }
    }

    game_input Input[2] = {};
    game_input *NewInput = &Input[0];
    game_input *OldInput = &Input[1];

    // Game var inits 
    game_state GameState = {};

    // Init timer for enforcing FPS
    int MonitorRefreshHz = 60;
    int GameUpdateHz = MonitorRefreshHz/2;
    real32 TargetTimePerFrame = 1.0f / (real32)GameUpdateHz;

    GlobalPerformanceFrequency = SDL_GetPerformanceFrequency();
    uint64 StartCounter = SDL_GetPerformanceCounter();

    //SDL_Event Event;
    while(GlobalRunning) {
        game_controller_state *OldKeyboardController = GetController(OldInput, 0);
        game_controller_state *NewKeyboardController = GetController(NewInput, 0);
        *NewKeyboardController = {};
        NewKeyboardController->IsConnected = true;

        for(int ButtonIndex =0;
            ButtonIndex < ArrayLength(NewKeyboardController->Buttons);
            ++ButtonIndex)
        {
            NewKeyboardController->Buttons[ButtonIndex] =
                OldKeyboardController->Buttons[ButtonIndex];
        }

        SDLEventHandler(NewKeyboardController);

        for(int ControllerIndex = 0; ControllerIndex < ControllerCount; ++ControllerIndex) {
            // TODO(Vincent): Fix controller unplugged then plugged back in

            int OurControllerIndex = ControllerIndex + 1;
            //game_controller_state *OldController = GetController(OldInput, OurControllerIndex);
            game_controller_state *NewController = GetController(NewInput, OurControllerIndex);

            SDLProcessControllerInput(GameController[ControllerIndex], NewController);

        } 

        game_offscreen_buffer Buffer = {};
        Buffer.Pixels = offscreen_buffer.Pixels;
        Buffer.Height = offscreen_buffer.Height;
        Buffer.Width = offscreen_buffer.Width;
        Buffer.Pitch = offscreen_buffer.Pitch;

        GameUpdateAndRender(&Buffer, NewInput, &GameState);

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
            //OutputDebugStringA("MISSED FRAME\n");
        }

        // Show frame on screen after state update
        SDLDisplayBufferInWindow(&offscreen_buffer);

        real32 MSPerFrame = 1000.0f*SDLGetSecondsElapsed(StartCounter, EndCounter);

        game_input *Temp = NewInput;
        NewInput = OldInput;
        OldInput = Temp;

        char FPSBuffer[256];
        _snprintf_s(FPSBuffer, sizeof(FPSBuffer),
                    "%.02fms/f\n", MSPerFrame);
        //OutputDebugStringA(FPSBuffer);

        StartCounter = EndCounter;
    }

    SDL_Quit();

	return 0;
}
