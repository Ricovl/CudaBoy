#pragma once
#include <SDL2/SDL.h>
#include <chrono>

#include "../gameboy/LR35902.hpp"
#include "../gameboy/rom.hpp"

typedef std::chrono::high_resolution_clock Clock;

#define GB_SCREEN_HEIGHT 144
#define GB_SCREEN_WIDTH  160

#define GB_TILEDATA_HEIGHT 16*16
#define GB_TILEDATA_WIDTH  16*8


int main(int argc, const char* argv[])
{
    bool quit = false;
    SDL_Event event;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("SDL2 Starter Project",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (GB_TILEDATA_WIDTH * 2) + (GB_SCREEN_WIDTH * 2), GB_TILEDATA_HEIGHT * 2, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Surface* window_surface = SDL_GetWindowSurface(window);

    // SDL_Texture* gb0_texture = SDL_CreateTexture(renderer, SDL_BITSPERPIXEL(2), SDL_TEXTUREACCESS_STREAMING, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT);
    SDL_Texture* gb0_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT);
    SDL_Texture* tiledata_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, GB_TILEDATA_WIDTH, GB_TILEDATA_HEIGHT);

    SDL_Rect rect;
    rect.w = GB_SCREEN_WIDTH * 2;
    rect.h = GB_SCREEN_HEIGHT * 2;
    rect.x = 0;
    rect.y = 0;
    SDL_Rect rectz;
    rect.w = GB_SCREEN_WIDTH;
    rect.h = GB_SCREEN_HEIGHT;
    rect.x = 0;
    rect.y = 0;
    SDL_Rect rect1;
    rect1.w = GB_TILEDATA_WIDTH;
    rect1.h = GB_TILEDATA_HEIGHT;
    rect1.x = GB_SCREEN_WIDTH - 0;
    rect1.y = 0;
    SDL_Rect rect1z;
    rect1.w = GB_TILEDATA_WIDTH * 2;
    rect1.h = GB_TILEDATA_HEIGHT * 2;
    rect1.x = GB_SCREEN_WIDTH * 2;
    rect1.y = 0;

    unsigned int gb0_vram[GB_SCREEN_HEIGHT * GB_SCREEN_WIDTH * 4];
    unsigned int tiledata_buf[GB_TILEDATA_HEIGHT * GB_TILEDATA_WIDTH * 4];

    auto prevTime = Clock::now();
    auto prevTicks = SDL_GetTicks();

    uint8_t *rom;
    load_rom(&rom);
    state_t *gb_state;
    initialize_state(gb_state, rom);
    std::cout << "rom size " << gb_state->rom_size << "\n";


    while (!quit)
    {
        auto now = Clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>(now - prevTime).count();
        if (delta > 239) {
            if (!gb_state->halt) {
                step(*gb_state);
            }
        }

        auto currentTicks = SDL_GetTicks();
        if (currentTicks - prevTicks >= 10) {
            SDL_PollEvent(&event);

            switch (event.type)
            {
                case SDL_QUIT:
                    quit = true;
                    break;
            }

            // memset(gb0_vram, 0xff00ffff, GB_SCREEN_HEIGHT * GB_SCREEN_WIDTH);
                    // Set every pixel to white.
            uint8_t i = 0;
            for (int y = 0; y < GB_SCREEN_HEIGHT; ++y)
            {
                    i++;
                    uint32_t color = 0x000000ff;
                    ((uint8_t *)&color)[0] = i;
                for (int x = 0; x < GB_SCREEN_WIDTH; ++x)
                {
                    gb0_vram[x + (y * GB_SCREEN_WIDTH)] = color;
                }
            }

            i = 0;
            for (int y = 0; y < GB_TILEDATA_HEIGHT; y += 8)
            {
                for (int x = 0; x < GB_TILEDATA_WIDTH; x += 8)
                {
                    unsigned addr = 0x8000 + (i * 16);

                    for (int ys = 0; ys < 8; ys++) {
                        uint16_t row = read_u16(*gb_state, addr);
                        uint8_t b1 = read_u8(*gb_state, addr);
                        uint8_t b2 = read_u8(*gb_state, addr+1);
                        for (int xs = 0; xs < 8; xs++)
                        {
                            uint32_t color = 0xff000000;
                            uint8_t mask = 0x80>>xs;
                            uint8_t px_col = (!!(b2 & mask) << 1) | !!(b1 & mask);

                            if (px_col == 0b00)
                                color = 0xfff4fff4;
                            if (px_col == 0b01)
                                color = 0xffc0d0c0;
                            if (px_col == 0b10)
                                color = 0xff80a080;
                            if (px_col == 0b11)
                                color = 0xff001000;

                            tiledata_buf[(x + xs) + ((y + ys)  * GB_TILEDATA_WIDTH)] = color;
                        }
                        addr += 2;
                    }

                    i++;
                }
            }

            SDL_RenderClear(renderer);
            
            SDL_Rect dest_rect = {0, 0, GB_SCREEN_WIDTH * 2, GB_SCREEN_HEIGHT * 2};
            SDL_UpdateTexture(gb0_texture, NULL, gb0_vram, GB_SCREEN_WIDTH * 4);
            SDL_RenderCopy(renderer, gb0_texture, NULL, &dest_rect);

            SDL_Rect tile_rect = {dest_rect.w, 0, GB_TILEDATA_WIDTH * 2, GB_TILEDATA_HEIGHT * 2};
            SDL_UpdateTexture(tiledata_texture, NULL, tiledata_buf, GB_TILEDATA_WIDTH * 4);
            SDL_RenderCopy(renderer, tiledata_texture, NULL, &tile_rect);

            SDL_RenderPresent(renderer);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
