#pragma once

#include <cstdio>
#include "config.hpp"

enum
{
    HBLANK = 0,
    VBLANK = 1,
    OAM = 2,
    OAMRAM = 3,
};

struct lcd_t {

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

    uint16_t win_tilemap_addr;
    uint16_t bg_tilemap_addr;
    uint16_t bg_tiledata_addr;

    uint8_t mode;
    unsigned cycles;
    uint8_t ly;

    uint8_t vram[144 * 160];
};

