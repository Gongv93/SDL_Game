#ifndef GAME_H
#define GAME_H

#define ArrayLength(Array) (sizeof(Array) / sizeof((Array)[0]))

struct game_state
{
    int BlueOffset;
    int GreenOffset;
};

struct game_offscreen_buffer
{
    void *Pixels;
    int Height;
    int Width;
    int Pitch;
};


struct game_controller_state
{
    bool32 IsConnected;
    bool32 IsAnalog;    
    real32 StickAverageX;
    real32 StickAverageY;

    union
    {
        bool32 Buttons[12];
        struct
        {
            bool32 MoveUp;
            bool32 MoveDown;
            bool32 MoveLeft;
            bool32 MoveRight;

            bool32 ActionUp;
            bool32 ActionDown;
            bool32 ActionLeft;
            bool32 ActionRight;

            bool32 LeftShoulder;
            bool32 RightShoulder;

            bool32 Back;
            bool32 Start;
        };
    };
};

struct game_input
{
    // NOTE(Vincent): Controller[0] : Keyboard,
    //                Controller[1-4] : Controller
    game_controller_state Controllers[5];
};

inline game_controller_state *
GetController(game_input *Input, int unsigned ControllerIndex) {
    game_controller_state *Result = &Input->Controllers[ControllerIndex];
    return Result;
}

#endif