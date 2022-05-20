#pragma once

#include "lcd_state.hpp"
#include "state.hpp"

static void lcd_cycle(state_t&s) {

    switch (s.lcd.mode)
    {
    case HBLANK:  // 204 clock cycles
        break;
    case OAM:     // 80 clock cycles
        break;
    case OAMRAM:  // 172 clock cycles
        break;
    case VBLANK:  // 4560 clock cycles
        break;
    }
    if (s.lcd.cycles >= 456) {
        s.lcd.cycles = 0;
        s.lcd.ly += 1;

        if (s.lcd.ly > 153) {
            s.lcd.ly = 0;
            s.lcd.mode = OAM;
        }
        else if (s.lcd.ly > 143) {
            s.lcd.mode = VBLANK;
        }
    }

    s.lcd.cycles += 1;
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

static void lcd_init(state_t &s) {
    lcd_t &lcd = s.lcd;

    memset(&lcd, 0, sizeof(lcd_t));
}