#ifndef MAIN_PLATFORM_H
#define MAIN_PLATFORM_H

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

#endif