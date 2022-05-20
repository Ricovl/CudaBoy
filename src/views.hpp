#pragma once

#include <SDL2/SDL.h>

#include "../gameboy/state.hpp"

#define GB_SCREEN_HEIGHT 144
#define GB_SCREEN_WIDTH  160

#define GB_TILEDATA_HEIGHT 16*16
#define GB_TILEDATA_WIDTH  16*8

#define GB_BG_HEIGHT (32*8 + 33)
#define GB_BG_WIDTH  (32*8 + 33)

#define GB_WINDOW_HEIGHT 70
#define GB_WINDOW_WIDTH  80


struct debug_view_t {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    unsigned scale;
};

int create_view(debug_view_t &view, const char *title, unsigned width, unsigned height, unsigned scale);

int update_screen_view(debug_view_t &view, state_t &s);
int update_tilemap_view(debug_view_t &view, state_t &s);
int update_bg_view(debug_view_t &view, state_t &s);