#pragma once

#include <cstdint>
#include "config.hpp"
#include "lcd_ctrl.hpp"

static unsigned const char bootrom[256] =
    {
        0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
        0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
        0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
        0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
        0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
        0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
        0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
        0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
        0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
        0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
        0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
        0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
        0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
        0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C,
        0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
        0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50};

#define CARDTRIDGE_ROM_START  0x0100
#define CARDTRIDGE_ROM_END    0x7fff

#define CHARACTER_RAM_START   0x8000
#define CHARACTER_RAM_END     0x97ff

#define BACKGROUND_MAP_DATA_START 0x9800
#define BACKGROUND_MAP_DATA_END   0x9fff

static uint8_t read_u8(state_t &s, _reg16_t ptr) {
#if USE_BOOTROM
    if (ptr < 0x100) {
        return bootrom[ptr];
    }
#endif

    switch (ptr)
    {
        case P1:
            return 0xff;
            break;
        case LCDC:
            printf("reading lcdc\n");
            break;
        case LY: // LCDC (lcd control register)
            // printf("reading ly\n");
            return s.lcd.ly;
            // return 0x90;
            break;

        default:
            break;
    }

    if (ptr >= 0xfea0 && ptr <= 0xfeff) {
        return 0xff;
    }

    return s.mem[ptr];
}

static uint16_t read_u16(state_t &s, _reg16_t ptr) {
    return read_u8(s, ptr) | (read_u8(s, ptr + 1) << 8);
}

static void write_u8(state_t &s, _reg16_t ptr, uint8_t n) {
    uint16_t tac_values[] = {1024, 16, 64, 256};
    uint16_t dest;

    if (ptr < 0x8000)
        return;

    switch (ptr)
    {
    case 0xff02:
        if (n == 0x81) {
            char c = read_u8(s, 0xff01);
            printf("%c", c);
            n = 0;
        }
        break;
    case DIV:
        s.divider_counter = 0;
        n = 0;
        break;
    case TAC:
        s.timer_tac = tac_values[n & 3];
        s.timer_enable = (n >> 2) & 1;
        // printf("timer enable: %d at speed %d\n", s.timer_enable, s.timer_tac);
        break;
    case LCDC:
        lcd_control_set(s, n);
        break;
    case IE:
        printf("enabling interrupt.. \n");
        if (n & (1 << 0)) {
            printf("Vblank ");
        }
        if (n & (1 << 1)) {
            printf("LCD_stat ");
        }
        if (n & (1 << 2)) {
            printf("Timer ");
        }
        printf("ie: %d ", s.interrupts_enabled);
        printf("\n");
        break;
    case IF:
        // if (s.interrupts_enabled) {
        //     printf("triggered interrupt: \n");
        //     if (n & (1 << 0)) {
        //         printf("Vblank ");
        //     }
        //     if (n & (1 << 1)) {
        //         printf("LCD_stat ");
        //     }
        //     if (n & (1 << 2)) {
        //         printf("Timer ");
        //     }
        //     printf("ie: %d ", s.interrupts_enabled);
        //     printf("\n");
        // }
        break;
    case DMA:
        // printf("dma transfer request!!!\n");
        dest = ((n & 0x00ff) << 8);
        memcpy(&s.mem[0xfe00], &s.mem[dest], 0xfe9f - 0xfe00);

    default:
        break;
    }

    s.mem[ptr] = n;
}

static void write_u16(state_t &s, _reg16_t ptr, uint16_t n) {
    write_u8(s, ptr, (uint8_t)(n & 0x00ff));
    write_u8(s, ptr + 1, (uint8_t)((n & 0xff00) >> 8));
}