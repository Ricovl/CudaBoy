#pragma once

#include "state.hpp"

enum
{
    HBLANK = 0,
    VBLANK = 1,
    OAM = 2,
    OAMRAM = 3,
};

typedef struct lcd_t {

    struct {
        uint8_t lcd_enable : 1;

        uint8_t window_tilemap_select : 1;
        uint8_t window_enable : 1;

        uint8_t bg_tiledata_select : 1;
        uint8_t bg_tilemap_select : 1;

        uint8_t sprite_size : 1;
        uint8_t sprites_enable : 1;

        uint8_t bg_enable : 1;
    };

    uint8_t mode;
    unsigned cycles;
};

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

}

static void lcd_init(state_t &s) {

}