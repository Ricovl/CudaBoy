#pragma once

#include <cstdint>
#include <cmath>
#include "lcd_state.hpp"
#include "interrupts.hpp"

static uint8_t read_u8(state_t &s, _reg16_t ptr);

static uint8_t lcd_get_bg_pixel(state_t &s, uint8_t x, uint8_t y) {
    uint8_t scy = read_u8(s, 0xff42);
    uint8_t scx = read_u8(s, 0xff43);

    uint8_t bgy = (scy + y) % 0xff;
    uint8_t bgx = (scx + x) % 0xff;

    uint8_t tilex = floor(bgx / 8);
    uint8_t tiley = floor(bgy / 8);

    uint16_t tilenum_addr = s.lcd.bg_tilemap_addr + tilex + (tiley * 32);
    uint8_t tilenum = read_u8(s, tilenum_addr);

    uint16_t tiledata_addr = s.lcd.bg_tiledata_addr;
    if (s.lcd.bg_tiledata_select) {
        tiledata_addr += tilenum * 16;
    }
    else {
        tiledata_addr += (int8_t)tilenum * 16;
    }

    tiledata_addr += 2 * (bgy % 8);
    uint8_t b1 = read_u8(s, tiledata_addr);
    uint8_t b2 = read_u8(s, tiledata_addr + 1);

    uint8_t mask = 0x80 >> (bgx % 8);
    uint8_t px_col = (!!(b2 & mask) << 1) | !!(b1 & mask);
    return px_col;
}

static void lcd_draw_bg_line(state_t &s, uint8_t ly) {
    for (uint8_t x = 0; x < 160; x++) {
        uint8_t px = lcd_get_bg_pixel(s, x, ly);
        s.lcd.vram[x + (ly * 160)] = px;
    }
}

struct alignas(1) object_t 
{
    uint8_t y;
    uint8_t x;
    uint8_t tile;
    union {
        struct alignas(1) {
            uint8_t cgb_palette : 3;
            uint8_t bank : 1;
            uint8_t palette : 1;
            uint8_t x_flip : 1;
            uint8_t y_flip : 1;
            uint8_t bg_priority : 1;
        };
        uint8_t flag;
    };
};

static void lcd_draw_sprites_line(state_t &s, uint8_t ly) {
    int sprites_valid = 0;

    if(!s.lcd.sprites_enable) {
        return;
    }
    uint8_t height = (s.lcd.sprite_size * 8) + 8;

    for (int i = 0; i < 40; i++) {
        object_t *sprite = (object_t*)(s.mem + SPRITE_ATTRIBUTE_TABLE + (i * 4));
        // uint16_t sprite_ptr = SPRITE_ATTRIBUTE_TABLE + (i * 4);

        int16_t sprite_y = (int16_t)sprite->y - 16;
        int16_t sprite_x = (int16_t)sprite->x - 8;
        // uint8_t sprite_i = read_u8(s, sprite_ptr + 2);
        // uint8_t sprite_f = read_u8(s, sprite_ptr + 3);

        if (ly >= sprite_y && ly < (sprite_y + height)) {
            sprites_valid++;

            uint16_t tiledata_addr = SPRITE_TILES_TABLE + (sprite->tile * 16);

            uint8_t tile_py = (ly - sprite_y);
            if (sprite->y_flip) {
                tile_py = (height - 1) - tile_py;
            }
            tiledata_addr += 2 * tile_py;
            uint8_t b1 = read_u8(s, tiledata_addr);
            uint8_t b2 = read_u8(s, tiledata_addr + 1);

            for (uint8_t x = 0; x < 8; x++) {
                uint8_t mask = 0x80 >> x;
                if (sprite->x_flip) {
                    mask = 0x80 >> (7 - x);
                }
                uint8_t px_col = (!!(b2 & mask) << 1) | !!(b1 & mask);

                if (px_col && (sprite_x + x) >= 0 && (sprite_x + x) < 160) {
                    s.lcd.vram[(sprite_x + x) + (ly * 160)] = px_col;
                }
            }
        }
    }

}

static void lcd_cycle(state_t &s) {
    lcd_stat_t status;
    status.raw = read_u8(s, STAT);
    status.lyc_eq_ly = 0;

    s.lcd.cycles += 1;

    switch (s.lcd.mode)
    {
    case lcd::OAM:     // 80 clock cycles   OAM Search
        if (s.lcd.cycles >= 80) {
            s.lcd.mode = lcd::OAMRAM;
        }
        break;
    case lcd::OAMRAM:  // 172 clock cycles  Pixel Transfer
        if (s.lcd.cycles >= 80 + 172) {
            s.lcd.mode = lcd::HBLANK;

            lcd_draw_bg_line(s, s.lcd.ly);
            lcd_draw_sprites_line(s, s.lcd.ly);

            if (status.hblank_int) {
                interrupt_trigger(s, Int::LCD_STAT);
            }
        }
        break;
    case lcd::HBLANK:  // 204 clock cycles  H-Blank
        if (s.lcd.cycles >= 80 + 172 + 204) {
            s.lcd.cycles = 0;
            s.lcd.ly += 1;

            s.lcd.mode = lcd::OAM;
            if (s.lcd.ly >= 144)
            {
                s.lcd.mode = lcd::VBLANK;
                interrupt_trigger(s, Int::VBLANK);
                if (status.vblank_int) {
                    interrupt_trigger(s, Int::LCD_STAT);
                }
            }

            if (s.lcd.ly == read_u8(s, LYC)) {
                status.lyc_eq_ly = 1;
                interrupt_trigger(s, Int::LCD_STAT);
            }
        }
        break;
    case lcd::VBLANK:  // 4560 clock cycles V-Blank
        if (s.lcd.cycles >= 80 + 172 + 204) {
            s.lcd.cycles = 0;
            s.lcd.ly += 1;

            if (s.lcd.ly >= 154) {
                s.lcd.ly = 0;
                s.lcd.mode = lcd::OAM;
            }

            if (s.lcd.ly == read_u8(s, LYC)) {
                status.lyc_eq_ly = 1;
                interrupt_trigger(s, Int::LCD_STAT);
            }
        }
        break;
    }

    status.mode = s.lcd.mode & 3;
    write_u8(s, STAT, status.raw);
}

static void lcd_init(state_t &s) {
    lcd_t &lcd = s.lcd;

    memset(&lcd, 0, sizeof(lcd_t));
}

static void lcd_control_set(state_t &s, uint8_t lcdc) {
    s.lcd.lcd_enable            = (lcdc >> 7) & 0x01;
    s.lcd.window_tilemap_select = (lcdc >> 6) & 0x01;
    s.lcd.window_enable         = (lcdc >> 5) & 0x01;
    s.lcd.bg_tiledata_select    = (lcdc >> 4) & 0x01;
    s.lcd.bg_tilemap_select     = (lcdc >> 3) & 0x01;
    s.lcd.sprite_size           = (lcdc >> 2) & 0x01;
    s.lcd.sprites_enable        = (lcdc >> 1) & 0x01;
    s.lcd.bg_enable             = (lcdc >> 0) & 0x01;

    s.lcd.win_tilemap_addr = !s.lcd.window_tilemap_select ? 0x9800 : 0x9c00;
    s.lcd.bg_tilemap_addr = !s.lcd.bg_tilemap_select ? 0x9800 : 0x9c00;
    s.lcd.bg_tiledata_addr = s.lcd.bg_tiledata_select ? 0x8000 : 0x9000;
}