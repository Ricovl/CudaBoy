
#include "views.hpp"

#include "../gameboy/mem.hpp"

int create_view(debug_view_t &view, const char *title, unsigned width, unsigned height, unsigned scale)
{
    view.scale = scale;
    view.window = SDL_CreateWindow(title,
                                   SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width * scale, height * scale, 0);
    view.renderer = SDL_CreateRenderer(view.window, -1, 0);
    view.texture = SDL_CreateTexture(view.renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, width, height);
}

int update_screen_view(debug_view_t &view, state_t &s)
{
    unsigned int gb0_vram[GB_SCREEN_HEIGHT * GB_SCREEN_WIDTH * 4];
    uint8_t i = 0;

    for (int y = 0; y < GB_SCREEN_HEIGHT; ++y)
    {
        i++;
        uint32_t color = 0x00ff00ff;
        ((uint8_t *)&color)[0] = i;
        uint8_t j = 0;
        for (int x = 0; x < GB_SCREEN_WIDTH; ++x)
        {
            ((uint8_t *)&color)[2] = j;
            gb0_vram[x + (y * GB_SCREEN_WIDTH)] = color;
            j++;
        }
    }

    SDL_RenderClear(view.renderer);

    SDL_UpdateTexture(view.texture, NULL, gb0_vram, GB_SCREEN_WIDTH * 4);
    SDL_RenderCopy(view.renderer, view.texture, NULL, NULL);

    SDL_RenderPresent(view.renderer);
}

int update_tilemap_view(debug_view_t &view, state_t &s)
{
    unsigned int tiledata_buf[GB_TILEDATA_HEIGHT * GB_TILEDATA_WIDTH * 4];
    uint8_t i = 0;

    for (int y = 0; y < GB_TILEDATA_HEIGHT; y += 8)
    {
        for (int x = 0; x < GB_TILEDATA_WIDTH; x += 8)
        {
            unsigned addr = 0x8000 + (i * 16);

            for (int ys = 0; ys < 8; ys++)
            {
                uint16_t row = read_u16(s, addr);
                uint8_t b1 = read_u8(s, addr);
                uint8_t b2 = read_u8(s, addr + 1);
                for (int xs = 0; xs < 8; xs++)
                {
                    uint32_t color = 0xff000000;
                    uint8_t mask = 0x80 >> xs;
                    uint8_t px_col = (!!(b2 & mask) << 1) | !!(b1 & mask);

                    if (px_col == 0b00)
                        color = 0xfff4fff4;
                    if (px_col == 0b01)
                        color = 0xffc0d0c0;
                    if (px_col == 0b10)
                        color = 0xff80a080;
                    if (px_col == 0b11)
                        color = 0xff001000;

                    tiledata_buf[(x + xs) + ((y + ys) * GB_TILEDATA_WIDTH)] = color;
                }
                addr += 2;
            }

            i++;
        }
    }

    SDL_RenderClear(view.renderer);

    SDL_UpdateTexture(view.texture, NULL, tiledata_buf, GB_TILEDATA_WIDTH * 4);
    SDL_RenderCopy(view.renderer, view.texture, NULL, NULL);

    SDL_RenderPresent(view.renderer);
}

int update_bg_view(debug_view_t &view, state_t &s)
{
    unsigned int buf[GB_BG_HEIGHT * GB_BG_WIDTH * 4];

    uint8_t scy = read_u8(s, 0xff42);
    uint8_t scx = read_u8(s, 0xff43);
    printf("%02x\n", scy);
    SDL_Rect rect = {scx * view.scale + ((scx / 2) * 2),
                     scy * view.scale + 2,
                     GB_SCREEN_WIDTH * view.scale,
                     GB_SCREEN_HEIGHT * view.scale};

    uint8_t wy = read_u8(s, 0xff4A);
    uint8_t wx = read_u8(s, 0xff4B) - 7;
    SDL_Rect wrect = {scx + wx, scy + wy, GB_WINDOW_WIDTH * view.scale, GB_WINDOW_HEIGHT * view.scale};

    int bw = 1 + (GB_SCREEN_WIDTH / 8) * (8 + 1);
    int bh = 1 + (GB_SCREEN_HEIGHT / 8) * (8 + 1);
    int by = scy + (ceil(scy / 8) * 1);
    int bx = scx + (ceil(scy / 8) * 1);
    wmemset((wchar_t *)buf, 0xff8c8c8c, GB_BG_HEIGHT * GB_BG_WIDTH);

    SDL_RenderClear(view.renderer);

    {
        unsigned addr = s.lcd.bg_tilemap_addr;

        // printf("map\n");
        for (int y = 1; y < GB_BG_WIDTH; y += 8 + 1)
        {
            for (int x = 1; x < GB_BG_HEIGHT; x += 8 + 1)
            {
                uint8_t tile = read_u8(s, addr);
                // printf("%02x ", tile);
                unsigned tile_addr = s.lcd.bg_tiledata_addr;
                if (s.lcd.bg_tiledata_select)
                    tile_addr += (tile * 16);
                else
                    tile_addr += ((int8_t)tile * 16);

                for (int ys = 0; ys < 8; ys++)
                {
                    uint16_t row = read_u16(s, tile_addr);
                    uint8_t b1 = read_u8(s, tile_addr);
                    uint8_t b2 = read_u8(s, tile_addr + 1);
                    for (int xs = 0; xs < 8; xs++)
                    {
                        uint32_t color = 0xff000000;
                        uint8_t mask = 0x80 >> xs;
                        uint8_t px_col = (!!(b2 & mask) << 1) | !!(b1 & mask);

                        if (px_col == 0b00)
                        {
                            color = 0xfff4fff4;
                        }
                        if (px_col == 0b01)
                            color = 0xffc0d0c0;
                        if (px_col == 0b10)
                            color = 0xff80a080;
                        if (px_col == 0b11)
                            color = 0xff001000;

                        buf[(x + xs) + ((y + ys) * GB_BG_WIDTH)] = color;
                    }
                    tile_addr += 2;
                }

                addr += 1;
            }
            // printf("\n");
        }
    }
    wmemset((wchar_t *)(buf + (GB_BG_WIDTH * by)), 0xffff00ff, bw);
    if (by + bh >= GB_BG_HEIGHT)
        by = (by + bh) - GB_BG_HEIGHT;
    else
        by = by + bh;
    wmemset((wchar_t *)(buf + (GB_BG_WIDTH * by)), 0xffff00ff, bw);

    SDL_UpdateTexture(view.texture, NULL, buf, GB_BG_WIDTH * 4);
    SDL_RenderCopy(view.renderer, view.texture, NULL, NULL);
    SDL_SetRenderDrawColor(view.renderer, 160, 25, 145, 0xff);
    SDL_RenderDrawRect(view.renderer, &rect);
    if (wy <= 144 && wx <= 166)
        SDL_RenderDrawRect(view.renderer, &wrect);

    SDL_RenderPresent(view.renderer);
}