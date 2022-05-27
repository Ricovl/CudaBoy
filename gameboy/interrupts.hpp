#pragma once

#include "state.hpp"
#include "mem.hpp"

enum
{
    VBLANK   = 1 << 0,
    LCD_STAT = 1 << 1,
    TIMER    = 1 << 2,
    SERIAL   = 1 << 3,
    JOYPAD   = 1 << 4,
};

static void interrupt_trigger(state_t &s, uint8_t i) {
    uint8_t int_flag = read_u8(s, IF);
    write_u8(s, IF, int_flag | i);
}