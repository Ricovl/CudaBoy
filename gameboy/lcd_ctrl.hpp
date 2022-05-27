#pragma once

#include <cstdint>
#include <cmath>
#include "lcd_state.hpp"

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

static void lcd_cycle(state_t &s) {
    s.lcd.cycles += 1;

    switch (s.lcd.mode)
    {
    case OAM:     // 80 clock cycles   OAM Search
        if (s.lcd.cycles >= 80) {
            s.lcd.mode = OAMRAM;
        }
        break;
    case OAMRAM:  // 172 clock cycles  Pixel Transfer
        if (s.lcd.cycles >= 80 + 172) {
            s.lcd.mode = HBLANK;

            lcd_draw_bg_line(s, s.lcd.ly);
        }
        break;
    case HBLANK:  // 204 clock cycles  H-Blank
        if (s.lcd.cycles >= 80 + 172 + 204) {
            s.lcd.cycles = 0;
            s.lcd.ly += 1;

            s.lcd.mode = OAM;
            if (s.lcd.ly >= 144)
            {
                s.lcd.mode = VBLANK;
            }
        }
        break;
    case VBLANK:  // 4560 clock cycles V-Blank
        if (s.lcd.cycles >= 80 + 172 + 204) {
            s.lcd.cycles = 0;
            s.lcd.ly += 1;

            if (s.lcd.ly >= 154) {
                s.lcd.ly = 0;
                s.lcd.mode = OAM;
            }
        }
        break;
    }
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