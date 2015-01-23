#include "game.h"

internal void
RenderPixels(game_offscreen_buffer *Buffer, game_state *GameState)
{
    // 32 bits per pixel, 8 bits per color
    // xx RR GG BB

    int Width  = Buffer->Width;
    int Height = Buffer->Height;

    uint8 *Row = (uint8 *)Buffer->Pixels;
    for(int y = 0; y < Height; ++y) {
        uint32 *Pixel = (uint32 *) Row;
        for(int x = 0; x < Width; ++x) {
            uint8 Blue = (uint8)(x + GameState->BlueOffset);
            uint8 Green = (uint8)(y + GameState->GreenOffset);

            *Pixel++ = ((Green << 8) | Blue);
        }
        Row += Buffer->Pitch;
    }
}


internal void
GameUpdateAndRender(game_offscreen_buffer *Buffer, game_input *Input, 
                    game_state *GameState) 
{    
    for(int ControllerIndex = 0;
        ControllerIndex < ArrayLength(Input->Controllers);
        ++ControllerIndex)
    {
        game_controller_state *Controller = GetController(Input, ControllerIndex);
        if(Controller->IsAnalog) {
            GameState->BlueOffset += (int)(10.0f*Controller->StickAverageX);
            GameState->GreenOffset += (int)(10.0f*Controller->StickAverageY);
        }
        else {
            if(Controller->MoveLeft)
            {
                GameState->BlueOffset -= 10;
            }
            
            if(Controller->MoveRight)
            {
                GameState->BlueOffset += 10;
            }
        }

        if(Controller->ActionDown)
        {
            GameState->GreenOffset += 1;
        }
    }

    RenderPixels(Buffer, GameState);

}