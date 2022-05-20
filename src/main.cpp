#include <SDL2/SDL.h>
#include <chrono>

#include "views.hpp"

#include "../gameboy/LR35902.hpp"
#include "../gameboy/rom.hpp"

typedef std::chrono::high_resolution_clock Clock;



int main(int argc, const char* argv[])
{
    bool quit = false;
    SDL_Event event;

    SDL_Init(SDL_INIT_VIDEO);

    debug_view_t gb_view;
    create_view(gb_view, "GB Screen", GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT, 2);

    debug_view_t tl_view;
    debug_view_t bg_view;
    create_view(tl_view, "TileMap", GB_TILEDATA_WIDTH, GB_TILEDATA_HEIGHT, 2);
    create_view(bg_view, "Background", GB_BG_WIDTH, GB_BG_HEIGHT, 2);

    auto prevTime = Clock::now();
    auto prevTicks = SDL_GetTicks();

    uint8_t *rom;
    load_rom(&rom);
    state_t *gb_state;
    initialize_state(gb_state, rom);
    std::cout << "rom size " << gb_state->rom_size << "\n";

    uint8_t prev_lcd_mode = gb_state->lcd.mode;
    bool wait_refresh = false;
    while (!quit)
    {
        auto now = Clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>(now - prevTime).count();
        // if (delta > 239) {
            // if (!gb_state->halt) {
            //     step(*gb_state);
            // }
        // }
        if (!gb_state->halt && !wait_refresh) {
            step(*gb_state);
        }
        if (gb_state->lcd.mode == 2 && prev_lcd_mode == 0) {
            wait_refresh = true;
        }
        prev_lcd_mode = gb_state->lcd.mode;

        auto currentTicks = SDL_GetTicks();
        if (currentTicks - prevTicks >= 16) {
            SDL_PollEvent(&event);

            switch (event.type)
            {
                case SDL_QUIT:
                    quit = true;
                    break;
            }


            update_screen_view(gb_view, *gb_state);
            update_tilemap_view(tl_view, *gb_state);
            update_bg_view(bg_view, *gb_state);
            prevTicks = currentTicks;
            wait_refresh = false;
        }
    }

    SDL_DestroyRenderer(gb_view.renderer);
    SDL_DestroyRenderer(tl_view.renderer);
    SDL_DestroyWindow(gb_view.window);
    SDL_DestroyWindow(tl_view.window);
    SDL_Quit();
}
