#ifndef GAME_H
#define GAME_H
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
    bool32 MoveUp;
    bool32 MoveDown;
    bool32 MoveLeft;
    bool32 MoveRight;
};

#endif