#pragma once

#include "state.hpp"

#define STACK_START 0xfffe


static void push_reg16(state_t &s, _reg16_t r) {
    s.regs.sp -= 2;
    write_u16(s, s.regs.sp, r);
}

static void pop_reg16(state_t &s, _reg16_t &r) {
    r = read_u16(s, s.regs.sp);
    s.regs.sp += 2;
}