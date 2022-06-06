#pragma once

#include <cstdio>
#include "config.hpp"

namespace lcd
{
const uint8_t HBLANK = 0;
const uint8_t VBLANK = 1;
const uint8_t OAM = 2;
const uint8_t OAMRAM = 3;
}

#define SPRITE_TILES_TABLE 0x8000
#define SPRITE_ATTRIBUTE_TABLE 0xfe00

struct alignas(1) lcd_stat_t {
    union {
        struct {
            uint8_t mode : 2;
            uint8_t lyc_eq_ly : 1;
            uint8_t hblank_int : 1;
            uint8_t vblank_int : 1;
            uint8_t oam_int : 1;
            uint8_t ly_int : 1;
        };
        uint8_t raw;
    };
};

struct lcd_t {

    struct {
        uint8_t lcd_enable : 1;

        uint8_t window_tilemap_select : 1;
        uint8_t window_enable : 1;

        uint8_t bg_tiledata_select : 1;
        uint8_t bg_tilemap_select : 1;

        uint8_t sprite_size : 1;            // 0=8x8, 1=8x16
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

