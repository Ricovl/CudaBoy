#pragma once

#include "state.hpp"

static uint8_t read_u8(state_t &s, _reg16_t ptr);
static void write_u8(state_t &s, _reg16_t ptr, uint8_t n);
static void write_u16(state_t &s, _reg16_t ptr, uint16_t n);

namespace Int
{
const uint8_t VBLANK   = 1 << 0; // 0100 0000
const uint8_t LCD_STAT = 1 << 1; // 0100 1000
const uint8_t TIMER    = 1 << 2; // 0101 0000
const uint8_t SERIAL   = 1 << 3; // 0101 1000
const uint8_t JOYPAD   = 1 << 4; // 0110 0000
}

static void interrupt_trigger(state_t &s, uint8_t i) {
    uint8_t int_flag = read_u8(s, IF);
    write_u8(s, IF, int_flag | i);
}

static void interrupt_clear(state_t &s, uint8_t i) {
    uint8_t int_flag = read_u8(s, IF);
    write_u8(s, IF, int_flag & ~i);
}

static void interrupts_handle(state_t &s) {
    uint8_t int_flag = read_u8(s, IF);
    uint8_t int_enable = read_u8(s, IE);

    uint8_t int_fired = int_flag & int_enable & 0x1f;
    if (int_fired) {
        s.halt = false;
        if (s.interrupts_enabled) {
            s.interrupts_enabled = false;
            s.regs.sp -= 2;
            write_u16(s, s.regs.sp, s.pc);

            if (int_fired & Int::VBLANK) {
                interrupt_clear(s, Int::VBLANK);
                s.pc = 0x40;
            }
            else if (int_fired & Int::LCD_STAT) {
                interrupt_clear(s, Int::LCD_STAT);
                s.pc = 0x48;
            }
            else if (int_fired & Int::TIMER) {
                // printf("fired timer interrupt!\n");
                interrupt_clear(s, Int::TIMER);
                s.pc = 0x50;
            }
            else if (int_fired & Int::SERIAL) {
                interrupt_clear(s, Int::SERIAL);
                s.pc = 0x58;
            }
            else if (int_fired & Int::JOYPAD) {
                interrupt_clear(s, Int::JOYPAD);
                s.pc = 0x60;
            }
            s.inst_cycles_wait += 20;
            // consumes 20 cycles?
        }
    }
}