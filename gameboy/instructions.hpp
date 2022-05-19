#pragma once

#include <cstdint>
#include "LR35902.hpp"
#include "state.hpp"
#include "mem.hpp"
#include "stack.hpp"

typedef void InstFun(state_t &s);

typedef struct instruction_t {
    char const *name;
    uint8_t length;
    uint8_t cycles;
    uint8_t operands;
    InstFun *execute;
};

// 8-bit ALU

inline void _add_a_n(state_t &s, _reg8_t n) {
    uint16_t res = s.regs.a + n;

    s.regs.subtract = 0;
    s.regs.half_carry = ((res & 0x0f) < (n & 0x0f));
    s.regs.carry = (res >= 0x100);

    s.regs.a = (_reg8_t)res;
    s.regs.zero = (s.regs.a == 0);
}

inline void _adc_a_n(state_t &s, _reg8_t n) {
    // _add_a_n(s, n + s.regs.carry);
    // s.regs.carry = s.regs.a + n + s.regs.carry >= 0x100;
    // s.regs.half_carry = ((s.regs.a & 0xf) + (n & 0xf) + s.regs.carry) >= 0x10;
    uint16_t carry = s.regs.a + n + s.regs.carry >= 0x100;

    s.regs.subtract = 0;
    s.regs.half_carry = ((s.regs.a & 0x0f) + (n & 0xf) + s.regs.carry) >= 0x10;
    s.regs.a = s.regs.a + n + s.regs.carry;
    s.regs.carry = carry;
    s.regs.zero = !s.regs.a;
}

inline void _sub_a_n(state_t &s, _reg8_t n) {
    uint8_t res = s.regs.a - n;

    s.regs.zero = (res == 0);
    s.regs.subtract = 1;
    s.regs.half_carry = ((n & 0x0f) > (s.regs.a & 0x0f));
    s.regs.carry = (n > s.regs.a);

    s.regs.a = (_reg8_t)res;
}

inline void _sbc_a_n(state_t &s, _reg8_t n) {
    uint8_t carry = s.regs.carry;

    s.regs.half_carry = (((n & 0x0f) + carry) > (s.regs.a & 0x0f));
    s.regs.carry = (n + carry > s.regs.a);
    s.regs.subtract = 1;
    s.regs.a -= (carry + n);
    s.regs.zero = !s.regs.a;
}

inline void _and_a_n(state_t &s, _reg8_t n) {
    s.regs.a = s.regs.a & n;

    s.regs.zero = (s.regs.a == 0);
    s.regs.subtract = 0;
    s.regs.half_carry = 1;
    s.regs.carry = 0;
}

inline void _or_a_n(state_t &s, _reg8_t n) {
    s.regs.a = s.regs.a | n;

    s.regs.zero = (s.regs.a == 0);
    s.regs.subtract = 0;
    s.regs.half_carry = 0;
    s.regs.carry = 0;
}

inline void _xor_a_n(state_t &s, _reg8_t n) {
    s.regs.a = s.regs.a ^ n;

    s.regs.zero = (s.regs.a == 0);
    s.regs.subtract = 0;
    s.regs.half_carry = 0;
    s.regs.carry = 0;
}

inline void _cp_a_n(state_t &s, _reg8_t n) {
    s.regs.zero = (s.regs.a == n);
    s.regs.subtract = 1;
    s.regs.half_carry = ((n & 0x0f) > (s.regs.a & 0x0f));
    s.regs.carry = (s.regs.a < n);
}

inline void _inc_reg8(state_t &s, _reg8_t &r) {
    s.regs.half_carry = (r & 0x0f) == 0x0f;

    r++;

    s.regs.subtract = 0;
    s.regs.zero = (r == 0);
}

inline void _dec_reg8(state_t &s, _reg8_t &r) {
    r--;

    s.regs.half_carry = (r & 0x0f) == 0x0f;
    s.regs.subtract = 1;
    s.regs.zero = (r == 0);
}

// 16-Bit Arithmetic

inline void _add_hl_reg16(state_t &s, _reg16_t r) {
    uint32_t res = s.regs.hl + r;

    s.regs.subtract = 0;
    s.regs.half_carry = ((s.regs.hl & 0xfff) + (r & 0xfff) > 0xfff);
    s.regs.carry = (res & 0xffff0000) != 0;

    s.regs.hl = (_reg16_t)res;
}

inline void _add_sp_u8(state_t &s, int8_t r) {
    uint32_t res = s.regs.sp + r;

    s.regs.zero = 0;
    s.regs.subtract = 0;
    s.regs.half_carry = ((res & 0xf) < (s.regs.sp & 0xf));
    s.regs.carry = ((res & 0xff) < (s.regs.sp & 0xff));

    s.regs.sp = (_reg16_t)res;
}

// Miscellaneous instructions
inline uint8_t _swap_n(state_t &s, uint8_t n) {
    uint8_t res = (n << 4) | (n >> 4);
    
    s.regs.zero = (res == 0);
    s.regs.subtract = 0;
    s.regs.half_carry = 0;
    s.regs.carry = 0;

    return res;
}

inline void _di(state_t &s) {
    s.interrupts_enabled = false;
}

inline void _ei(state_t &s) {
    s.interrupts_enabled = true;
}

// Rotate & Shifts instructions

inline uint8_t _rlc_n(state_t &s, uint8_t n) {
    uint8_t carry = (n >> 7) & 0x01;
    uint8_t res = (n << 1) | carry;

    s.regs.zero = (res == 0);
    s.regs.subtract = 0;
    s.regs.half_carry = 0;
    s.regs.carry = carry;

    return res;
}

inline uint8_t _rl_n(state_t &s, uint8_t n) {
    uint8_t carry = (n >> 7) & 0x01;
    uint8_t res = (n << 1) | s.regs.carry;

    s.regs.zero = (res == 0);
    s.regs.subtract = 0;
    s.regs.half_carry = 0;
    s.regs.carry = carry;

    return res;
}

inline uint8_t _rrc_n(state_t &s, uint8_t n) {
    uint8_t carry = n & 0x01;
    uint8_t res = (n >> 1) | (carry << 7);

    s.regs.zero = (res == 0);
    s.regs.subtract = 0;
    s.regs.half_carry = 0;
    s.regs.carry = carry;

    return res;
}

inline uint8_t _rr_n(state_t &s, uint8_t n) {
    uint8_t carry = n & 0x01;
    uint8_t res = (n >> 1) | (s.regs.carry << 7);

    s.regs.zero = (res == 0);
    s.regs.subtract = 0;
    s.regs.half_carry = 0;
    s.regs.carry = carry;

    return res;
}

inline uint8_t _sla_n(state_t &s, uint8_t n) {
    uint8_t carry = (n >> 7) & 0x01;
    uint8_t res = n << 1;

    s.regs.zero = (res == 0);
    s.regs.subtract = 0;
    s.regs.half_carry = 0;
    s.regs.carry = carry;

    return res;
}

inline uint8_t _sra_n(state_t &s, uint8_t n) {
    uint8_t msb = n & (0x01 << 7);
    uint8_t carry = n & 0x01;
    uint8_t res = (n >> 1) | msb;

    s.regs.zero = (res == 0);
    s.regs.subtract = 0;
    s.regs.half_carry = 0;
    s.regs.carry = carry;

    return res;
}

inline uint8_t _srl_n(state_t &s, uint8_t n) {
    uint8_t carry = n & 0x01;
    uint8_t res = n >> 1;

    s.regs.zero = (res == 0);
    s.regs.subtract = 0;
    s.regs.half_carry = 0;
    s.regs.carry = carry;

    return res;
}

// Bit Opcodes instructions

inline void _bit_b_r(state_t &s, uint8_t bit, _reg8_t r) {
    s.regs.zero = !(r & (0x01 << bit));
    s.regs.subtract = 0;
    s.regs.half_carry = 1;
}

inline uint8_t _set_b_r(state_t &s, uint8_t bit, uint8_t r) {
    return r | (0x01 << bit);
}

inline uint8_t _res_b_r(state_t &s, uint8_t bit, uint8_t r) {
    return r & ~(0x01 << bit);
}

// Jumps instructions

inline void _jr_cc_n(state_t &s, bool cc) {
    if (cc) {
        s.pc += (int8_t)s.operand;
        // printf("pc: %04x offset : %02X\n", s.pc, (int8_t)s.operand);
        // printf("jumping from %04x to %04x\n", s.prev_pc, s.pc);
        // branch taken so set cycles to right amount
        s.inst_cycles_wait = 12;
    }
}

inline void _jp_nn(state_t &s) {
    // std::cout << "jumping from " << s.pc << " to " << s.operand << "\n";
    s.pc = s.operand;
    // branch taken so set cycles to right amount
    s.inst_cycles_wait = 16;
}

inline void _call_nn(state_t &s) {
    push_reg16(s, s.pc);
    _jp_nn(s);
    // branch taken so set cycles to right amount
    s.inst_cycles_wait = 24;
}

inline void _rst_n(state_t &s, uint8_t n) {
    push_reg16(s, s.pc);
    s.pc = 0x0000 + n;
}


// =============================================================
// ============ Mapped Unprefixed instructions start ===========
// =============================================================

// 0x00
void nop(state_t &s) {
}

// 0x01
void ld_bc_nn(state_t &s) {
    s.regs.bc = s.operand;
}

// 0x02
void ld_bcp_a(state_t &s) {
    write_u8(s, s.regs.bc, s.regs.a);
}

// 0x03
void inc_bc(state_t &s) {
    s.regs.bc += 1;
}

// 0x04
void inc_b(state_t &s) {
    _inc_reg8(s, s.regs.b);
}

// 0x05
void dec_b(state_t &s) {
    _dec_reg8(s, s.regs.b);
}

// 0x06
void ld_b_n(state_t &s) {
    s.regs.b = (_reg8_t)s.operand;
}

// 0x07
void rlca(state_t &s) {
    s.regs.a = _rlc_n(s, s.regs.a);
    s.regs.zero = 0;
}

// 0x08
void ld_nnp_sp(state_t &s) {
    write_u16(s, s.operand, s.regs.sp);
}

// 0x09
void add_hl_bc(state_t &s) {
    _add_hl_reg16(s, s.regs.bc);
}

// 0x0a
void ld_a_bcp(state_t &s) {
    s.regs.a = read_u8(s, s.regs.bc);
}

// 0x0b
void dec_bc(state_t &s) {
    s.regs.bc -= 1;
}

// 0x0c
void inc_c(state_t &s) {
    _inc_reg8(s, s.regs.c);
}

// 0x0d
void dec_c(state_t &s) {
    _dec_reg8(s, s.regs.c);
}

// 0x0e
void ld_c_n(state_t &s) {
    s.regs.c = (_reg8_t)s.operand;
}

// 0x0f
void rrca(state_t &s) {
    s.regs.a = _rrc_n(s, s.regs.a);
    s.regs.zero = 0;
}

// 0x10
void stop(state_t &s) {
    s.stop = true;
}

// 0x11
void ld_de_nn(state_t &s) {
    s.regs.de = s.operand;
}

// 0x12
void ld_dep_a(state_t &s) {
    write_u8(s, s.regs.de, s.regs.a);
}

// 0x13
void inc_de(state_t &s) {
    s.regs.de += 1;
}

// 0x14
void inc_d(state_t &s) {
    _inc_reg8(s, s.regs.d);
}

// 0x15
void dec_d(state_t &s) {
    _dec_reg8(s, s.regs.d);
}

// 0x16
void ld_d_n(state_t &s) {
    s.regs.d = (_reg8_t)s.operand;
}

// 0x17
void rla(state_t &s) {
    s.regs.a = _rl_n(s, s.regs.a);
    s.regs.zero = 0;
}

// 0x18
void jr_n(state_t &s) {
    s.pc += (int8_t)s.operand;
}

// 0x19
void add_hl_de(state_t &s) {
    _add_hl_reg16(s, s.regs.de);
}

// 0x1a
void ld_a_dep(state_t &s) {
    s.regs.a = read_u8(s, s.regs.de);
}

// 0x1b
void dec_de(state_t &s) {
    s.regs.de -= 1;
}

// 0x1c
void inc_e(state_t &s) {
    _inc_reg8(s, s.regs.e);
}

// 0x1d
void dec_e(state_t &s) {
    _dec_reg8(s, s.regs.e);
}

// 0x1e
void ld_e_n(state_t &s) {
    s.regs.e = (_reg8_t)s.operand;
}

// 0x1f
void rra(state_t &s) {
    s.regs.a = _rr_n(s, s.regs.a);
    s.regs.zero = 0;
}


// 0x20
void jr_nz_n(state_t &s) {
    _jr_cc_n(s, !s.regs.zero);
}

// 0x21
void ld_hl_nn(state_t &s) {
    s.regs.hl = s.operand;
}

// 0x22
void ldi_hlp_a(state_t &s) {
    write_u8(s, s.regs.hl, s.regs.a);
    s.regs.hl += 1;
}

// 0x23
void inc_hl(state_t &s) {
    s.regs.hl += 1;
}

// 0x24
void inc_h(state_t &s) {
    _inc_reg8(s, s.regs.h);
}

// 0x25
void dec_h(state_t &s) {
    _dec_reg8(s, s.regs.h);
}

// 0x26
void ld_h_n(state_t &s) {
    s.regs.h = (_reg8_t)s.operand;
}

// 0x27
void daa(state_t &s) {
    _reg8_t a = s.regs.a;

    if (!s.regs.subtract) {
        if (s.regs.carry || a > 0x99) {
            a += 0x60;
            s.regs.carry = 1;
        }
        if (s.regs.half_carry | (a & 0x0f) > 0x09) {
            a += 0x06;
        }
    }
    else {
        if (s.regs.carry) {
            a -= 0x60;
        }
        if (s.regs.half_carry) {
            a -= 0x06;
        }
    }

    s.regs.zero = (a == 0);
    s.regs.half_carry = 0;
    s.regs.a = a;
}

// 0x28
void jr_z_n(state_t &s) {
    _jr_cc_n(s, s.regs.zero);
}

// 0x29
void add_hl_hl(state_t &s) {
    _add_hl_reg16(s, s.regs.hl);
}

// 0x2a
void ldi_a_hlp(state_t &s) {
    s.regs.a = read_u8(s, s.regs.hl);
    s.regs.hl += 1;
}

// 0x2b
void dec_hl(state_t &s) {
    s.regs.hl -= 1;
}

// 0x2c
void inc_l(state_t &s) {
    _inc_reg8(s, s.regs.l);
}

// 0x2d
void dec_l(state_t &s) {
    _dec_reg8(s, s.regs.l);
}

// 0x2e
void ld_l_n(state_t &s) {
    s.regs.l = (_reg8_t)s.operand;
}

// 0x2f
void cpl(state_t &s) {
    s.regs.a = ~s.regs.a;

    s.regs.half_carry = 1;
    s.regs.subtract = 1;
}


// 0x30
void jr_nc_n(state_t &s) {
    _jr_cc_n(s, !s.regs.carry);
}

// 0x31
void ld_sp_nn(state_t &s) {
    s.regs.sp = s.operand;
}

// 0x32
void ldd_hlp_a(state_t &s) {
    write_u8(s, s.regs.hl, s.regs.a);
    s.regs.hl -= 1;
}

// 0x33
void inc_sp(state_t &s) {
    s.regs.sp += 1;
}

// 0x34
void inc_hlp(state_t &s) {
    _reg8_t hlp = read_u8(s, s.regs.hl);
    _inc_reg8(s, hlp);
    write_u8(s, s.regs.hl, hlp);
}

// 0x35
void dec_hlp(state_t &s) {
    _reg8_t hlp = read_u8(s, s.regs.hl);
    _dec_reg8(s, hlp);
    write_u8(s, s.regs.hl, hlp);
}

// 0x36
void ld_hlp_n(state_t &s) {
    write_u8(s, s.regs.hl, (_reg8_t)s.operand);
}

// 0x37
void scf(state_t &s) {
    s.regs.subtract = 0;
    s.regs.half_carry = 0;
    s.regs.carry = 1;
}

// 0x38
void jr_c_n(state_t &s) {
    _jr_cc_n(s, s.regs.carry);
}

// 0x39
void add_hl_sp(state_t &s) {
    _add_hl_reg16(s, s.regs.sp);
}

// 0x3a
void ldd_a_hlp(state_t &s) {
    s.regs.a = read_u8(s, s.regs.hl);
    s.regs.hl -= 1;
}

// 0x3b
void dec_sp(state_t &s) {
    s.regs.sp -= 1;
}

// 0x3c
void inc_a(state_t &s) {
    _inc_reg8(s, s.regs.a);
}

// 0x3d
void dec_a(state_t &s) {
    _dec_reg8(s, s.regs.a);
}

// 0x3e
void ld_a_n(state_t &s) {
    s.regs.a = (_reg8_t)s.operand;
}

// 0x3f
void ccf(state_t &s) {
    s.regs.subtract = 0;
    s.regs.half_carry = 0;
    s.regs.carry ^= 1;
}


// 0x40
void ld_b_b(state_t &s) { s.regs.b = s.regs.b; }

// 0x41
void ld_b_c(state_t &s) { s.regs.b = s.regs.c; }

// 0x42
void ld_b_d(state_t &s) { s.regs.b = s.regs.d; }

// 0x43
void ld_b_e(state_t &s) { s.regs.b = s.regs.e; }

// 0x44
void ld_b_h(state_t &s) { s.regs.b = s.regs.h; }

// 0x45
void ld_b_l(state_t &s) { s.regs.b = s.regs.l; }

// 0x46
void ld_b_hlp(state_t &s) { s.regs.b = read_u8(s, s.regs.hl); }

// 0x47
void ld_b_a(state_t &s) { s.regs.b = s.regs.a; }

// 0x48
void ld_c_b(state_t &s) { s.regs.c = s.regs.b; }

// 0x49
void ld_c_c(state_t &s) { s.regs.c = s.regs.c; }

// 0x4a
void ld_c_d(state_t &s) { s.regs.c = s.regs.d; }

// 0x4b
void ld_c_e(state_t &s) { s.regs.c = s.regs.e; }

// 0x4c
void ld_c_h(state_t &s) { s.regs.c = s.regs.h; }

// 0x4d
void ld_c_l(state_t &s) { s.regs.c = s.regs.l; }

// 0x4e
void ld_c_hlp(state_t &s) { s.regs.c = read_u8(s, s.regs.hl); }

// 0x4f
void ld_c_a(state_t &s) { s.regs.c = s.regs.a; }

// 0x50
void ld_d_b(state_t &s) { s.regs.d = s.regs.b; }

// 0x51
void ld_d_c(state_t &s) { s.regs.d = s.regs.c; }

// 0x52
void ld_d_d(state_t &s) { s.regs.d = s.regs.d; }

// 0x53
void ld_d_e(state_t &s) { s.regs.d = s.regs.e; }

// 0x54
void ld_d_h(state_t &s) { s.regs.d = s.regs.h; }

// 0x55
void ld_d_l(state_t &s) { s.regs.d = s.regs.l; }

// 0x56
void ld_d_hlp(state_t &s) { s.regs.d = read_u8(s, s.regs.hl); }

// 0x57
void ld_d_a(state_t &s) { s.regs.d = s.regs.a; }

// 0x58
void ld_e_b(state_t &s) { s.regs.e = s.regs.b; }

// 0x59
void ld_e_c(state_t &s) { s.regs.e = s.regs.c; }

// 0x5a
void ld_e_d(state_t &s) { s.regs.e = s.regs.d; }

// 0x5b
void ld_e_e(state_t &s) { s.regs.e = s.regs.e; }

// 0x5c
void ld_e_h(state_t &s) { s.regs.e = s.regs.h; }

// 0x5d
void ld_e_l(state_t &s) { s.regs.e = s.regs.l; }

// 0x5e
void ld_e_hlp(state_t &s) { s.regs.e = read_u8(s, s.regs.hl); }

// 0x5f
void ld_e_a(state_t &s) { s.regs.e = s.regs.a; }

// 0x60
void ld_h_b(state_t &s) { s.regs.h = s.regs.b; }

// 0x61
void ld_h_c(state_t &s) { s.regs.h = s.regs.c; }

// 0x62
void ld_h_d(state_t &s) { s.regs.h = s.regs.d; }

// 0x63
void ld_h_e(state_t &s) { s.regs.h = s.regs.e; }

// 0x64
void ld_h_h(state_t &s) { s.regs.h = s.regs.h; }

// 0x65
void ld_h_l(state_t &s) { s.regs.h = s.regs.l; }

// 0x66
void ld_h_hlp(state_t &s) { s.regs.h = read_u8(s, s.regs.hl); }

// 0x67
void ld_h_a(state_t &s) { s.regs.h = s.regs.a; }

// 0x68
void ld_l_b(state_t &s) { s.regs.l = s.regs.b; }

// 0x69
void ld_l_c(state_t &s) { s.regs.l = s.regs.c; }

// 0x6a
void ld_l_d(state_t &s) { s.regs.l = s.regs.d; }

// 0x6b
void ld_l_e(state_t &s) { s.regs.l = s.regs.e; }

// 0x6c
void ld_l_h(state_t &s) { s.regs.l = s.regs.h; }

// 0x6d
void ld_l_l(state_t &s) { s.regs.l = s.regs.l; }

// 0x6e
void ld_l_hlp(state_t &s) { s.regs.l = read_u8(s, s.regs.hl); }

// 0x6f
void ld_l_a(state_t &s) { s.regs.l = s.regs.a; }

// 0x70
void ld_hlp_b(state_t &s) { write_u8(s, s.regs.hl, s.regs.b); }

// 0x71
void ld_hlp_c(state_t &s) { write_u8(s, s.regs.hl, s.regs.c); }

// 0x72
void ld_hlp_d(state_t &s) {write_u8(s, s.regs.hl, s.regs.d); }

// 0x73
void ld_hlp_e(state_t &s) { write_u8(s, s.regs.hl, s.regs.e); }

// 0x74
void ld_hlp_h(state_t &s) { write_u8(s, s.regs.hl, s.regs.h); }

// 0x75
void ld_hlp_l(state_t &s) { write_u8(s, s.regs.hl, s.regs.l); }

// 0x76
void halt(state_t &s) { s.halt = true; }

// 0x77
void ld_hlp_a(state_t &s) { write_u8(s, s.regs.hl, s.regs.a); }

// 0x78
void ld_a_b(state_t &s) { s.regs.a = s.regs.b; }

// 0x79
void ld_a_c(state_t &s) { s.regs.a = s.regs.c; }

// 0x7a
void ld_a_d(state_t &s) { s.regs.a = s.regs.d; }

// 0x7b
void ld_a_e(state_t &s) { s.regs.a = s.regs.e; }

// 0x7c
void ld_a_h(state_t &s) { s.regs.a = s.regs.h; }

// 0x7d
void ld_a_l(state_t &s) { s.regs.a = s.regs.l; }

// 0x7e
void ld_a_hlp(state_t &s) { s.regs.a = read_u8(s, s.regs.hl); }

// 0x7f
void ld_a_a(state_t &s) { s.regs.a = s.regs.a; }


// 0x80
void add_a_b(state_t &s) { _add_a_n(s, s.regs.b); }

// 0x81
void add_a_c(state_t &s) { _add_a_n(s, s.regs.c); }

// 0x82
void add_a_d(state_t &s) { _add_a_n(s, s.regs.d); }

// 0x83
void add_a_e(state_t &s) { _add_a_n(s, s.regs.e); }

// 0x84
void add_a_h(state_t &s) { _add_a_n(s, s.regs.h); }

// 0x85
void add_a_l(state_t &s) { _add_a_n(s, s.regs.l); }

// 0x86
void add_a_hlp(state_t &s) { _add_a_n(s, read_u8(s, s.regs.hl)); }

// 0x87
void add_a_a(state_t &s) { _add_a_n(s, s.regs.a); }

// 0x88
void adc_a_b(state_t &s) { _adc_a_n(s, s.regs.b); }

// 0x89
void adc_a_c(state_t &s) { _adc_a_n(s, s.regs.c); }

// 0x8a
void adc_a_d(state_t &s) { _adc_a_n(s, s.regs.d); }

// 0x8b
void adc_a_e(state_t &s) { _adc_a_n(s, s.regs.e); }

// 0x8c
void adc_a_h(state_t &s) { _adc_a_n(s, s.regs.h); }

// 0x8d
void adc_a_l(state_t &s) { _adc_a_n(s, s.regs.l); }

// 0x8e
void adc_a_hlp(state_t &s) { _adc_a_n(s, read_u8(s, s.regs.hl)); }

// 0x8f
void adc_a_a(state_t &s) { _adc_a_n(s, s.regs.a); }

// 0x90
void sub_a_b(state_t &s) { _sub_a_n(s, s.regs.b); }

// 0x91
void sub_a_c(state_t &s) { _sub_a_n(s, s.regs.c); }

// 0x92
void sub_a_d(state_t &s) { _sub_a_n(s, s.regs.d); }

// 0x93
void sub_a_e(state_t &s) { _sub_a_n(s, s.regs.e); }

// 0x94
void sub_a_h(state_t &s) { _sub_a_n(s, s.regs.h); }

// 0x95
void sub_a_l(state_t &s) { _sub_a_n(s, s.regs.l); }

// 0x96
void sub_a_hlp(state_t &s) { _sub_a_n(s, read_u8(s, s.regs.hl)); }

// 0x97
void sub_a_a(state_t &s) { _sub_a_n(s, s.regs.a); }

// 0x98
void sbc_a_b(state_t &s) { _sbc_a_n(s, s.regs.b); }

// 0x99
void sbc_a_c(state_t &s) { _sbc_a_n(s, s.regs.c); }

// 0x9a
void sbc_a_d(state_t &s) { _sbc_a_n(s, s.regs.d); }

// 0x9b
void sbc_a_e(state_t &s) { _sbc_a_n(s, s.regs.e); }

// 0x9c
void sbc_a_h(state_t &s) { _sbc_a_n(s, s.regs.h); }

// 0x9d
void sbc_a_l(state_t &s) { _sbc_a_n(s, s.regs.l); }

// 0x9e
void sbc_a_hlp(state_t &s) { _sbc_a_n(s, read_u8(s, s.regs.hl)); }

// 0x9f
void sbc_a_a(state_t &s) { _sbc_a_n(s, s.regs.a); }

// 0xa0
void and_a_b(state_t &s) { _and_a_n(s, s.regs.b); }

// 0xa1
void and_a_c(state_t &s) { _and_a_n(s, s.regs.c); }

// 0xa2
void and_a_d(state_t &s) { _and_a_n(s, s.regs.d); }

// 0xa3
void and_a_e(state_t &s) { _and_a_n(s, s.regs.e); }

// 0xa4
void and_a_h(state_t &s) { _and_a_n(s, s.regs.h); }

// 0xa5
void and_a_l(state_t &s) { _and_a_n(s, s.regs.l); }

// 0xa6
void and_a_hlp(state_t &s) { _and_a_n(s, read_u8(s, s.regs.hl)); }

// 0xa7
void and_a_a(state_t &s) { _and_a_n(s, s.regs.a); }

// 0xa8
void xor_a_b(state_t &s) { _xor_a_n(s, s.regs.b); }

// 0xa9
void xor_a_c(state_t &s) { _xor_a_n(s, s.regs.c); }

// 0xaa
void xor_a_d(state_t &s) { _xor_a_n(s, s.regs.d); }

// 0xab
void xor_a_e(state_t &s) { _xor_a_n(s, s.regs.e); }

// 0xac
void xor_a_h(state_t &s) { _xor_a_n(s, s.regs.h); }

// 0xad
void xor_a_l(state_t &s) { _xor_a_n(s, s.regs.l); }

// 0xae
void xor_a_hlp(state_t &s) { _xor_a_n(s, read_u8(s, s.regs.hl)); }

// 0xaf
void xor_a_a(state_t &s) { _xor_a_n(s, s.regs.a); }

// 0xb0
void or_a_b(state_t &s) { _or_a_n(s, s.regs.b); }

// 0xb1
void or_a_c(state_t &s) { _or_a_n(s, s.regs.c); }

// 0xb2
void or_a_d(state_t &s) { _or_a_n(s, s.regs.d); }

// 0xb3
void or_a_e(state_t &s) { _or_a_n(s, s.regs.e); }

// 0xb4
void or_a_h(state_t &s) { _or_a_n(s, s.regs.h); }

// 0xb5
void or_a_l(state_t &s) { _or_a_n(s, s.regs.l); }

// 0xb6
void or_a_hlp(state_t &s) { _or_a_n(s, read_u8(s, s.regs.hl)); }

// 0xb7
void or_a_a(state_t &s) { _or_a_n(s, s.regs.a); }

// 0xb8
void cp_a_b(state_t &s) { _cp_a_n(s, s.regs.b); }

// 0xb9
void cp_a_c(state_t &s) { _cp_a_n(s, s.regs.c); }

// 0xba
void cp_a_d(state_t &s) { _cp_a_n(s, s.regs.d); }

// 0xbb
void cp_a_e(state_t &s) { _cp_a_n(s, s.regs.e); }

// 0xbc
void cp_a_h(state_t &s) { _cp_a_n(s, s.regs.h); }

// 0xbd
void cp_a_l(state_t &s) { _cp_a_n(s, s.regs.l); }

// 0xbe
void cp_a_hlp(state_t &s) { _cp_a_n(s, read_u8(s, s.regs.hl)); }

// 0xbf
void cp_a_a(state_t &s) { _cp_a_n(s, s.regs.a); }


// 0xc0
void ret_nz(state_t &s) {
    if (!s.regs.zero) {
        pop_reg16(s, s.pc);
    }
}

// 0xc1
void pop_bc(state_t &s) { pop_reg16(s, s.regs.bc); }

// 0xc2
void jp_nz_nn(state_t &s) {
    if (!s.regs.zero) {
        _jp_nn(s);
    }
}

// 0xc3
void jp_nn(state_t &s) {
    _jp_nn(s);
}

// 0xc4
void call_nz_nn(state_t &s) {
    if (!s.regs.zero) {
        _call_nn(s);
    }
}

// 0xc5
void push_bc(state_t &s) {
    push_reg16(s, s.regs.bc);
}

// 0xc6
void add_a_n(state_t &s) {
    _add_a_n(s, (_reg8_t)s.operand);
}

// 0xc7
void rst_00h(state_t &s) {
    _rst_n(s, 0x00);
}

// 0xc8
void ret_z(state_t &s) {
    if (s.regs.zero) {
        pop_reg16(s, s.pc);
    }
}

// 0xc9
void ret(state_t &s) {
    pop_reg16(s, s.pc);
}

// 0xca
void jp_z_nn(state_t &s) {
    if (s.regs.zero) {
        _jp_nn(s);
    }
}

// 0xcb
void prefix_cb(state_t &s) {
    s.prefixed = true;
}

// 0xcc
void call_z_nn(state_t &s) {
    if (s.regs.zero) {
        _call_nn(s);
    }
}

// 0xcd
void call_nn(state_t &s) {
    _call_nn(s);
}

// 0xce
void adc_a_n(state_t &s) {
    _adc_a_n(s, (_reg8_t)s.operand);
}

// 0xcf
void rst_08h(state_t &s) {
    _rst_n(s, 0x08);
}



// 0xd0
void ret_nc(state_t &s) {
    if (!s.regs.carry) {
        pop_reg16(s, s.pc);
    }
}

// 0xd1
void pop_de(state_t &s) { pop_reg16(s, s.regs.de); }

// 0xd2
void jp_nc_nn(state_t &s) {
    if (!s.regs.carry) {
        _jp_nn(s);
    }
}

// 0xd4
void call_nc_nn(state_t &s) {
    if (!s.regs.carry) {
        _call_nn(s);
    }
}

// 0xd5
void push_de(state_t &s) {
    push_reg16(s, s.regs.de);
}

// 0xd6
void sub_a_n(state_t &s) {
    _sub_a_n(s, (_reg8_t)s.operand);
}

// 0xd7
void rst_10h(state_t &s) {
    _rst_n(s, 0x10);
}

// 0xd8
void ret_c(state_t &s) {
    if (s.regs.carry) {
        pop_reg16(s, s.pc);
    }
}

// 0xd9
void reti(state_t &s) {
    pop_reg16(s, s.pc);
    _ei(s);
}

// 0xda
void jp_c_nn(state_t &s) {
    if (s.regs.carry) {
        _jp_nn(s);
    }
}

// 0xdc
void call_c_nn(state_t &s) {
    if (s.regs.carry) {
        _call_nn(s);
    }
}

// 0xde
void sbc_a_n(state_t &s) {
    _sbc_a_n(s, (_reg8_t)s.operand);
}

// 0xdf
void rst_18h(state_t &s) {
    _rst_n(s, 0x18);
}


// 0xe0
void ldh_np_a(state_t &s) {
    write_u8(s, (_reg16_t)0xff00 + (_reg8_t)s.operand, s.regs.a);
}

// 0xe1
void pop_hl(state_t &s) { pop_reg16(s, s.regs.hl); }

// 0xe2
void ld_cp_a(state_t &s) {
    write_u8(s, (_reg16_t)0xff00 + (_reg8_t)s.regs.c, s.regs.a);
}

// 0xe5
void push_hl(state_t &s) {
    push_reg16(s, s.regs.hl); 
}

// 0xe6
void and_a_n(state_t &s) {
    _and_a_n(s, (_reg8_t)s.operand);
}

// 0xe7
void rst_20h(state_t &s) {
    _rst_n(s, 0x20);
}

// 0xe8
void add_sp_n(state_t &s) {
    _add_sp_u8(s, (_op8_t)s.operand);
}

// 0xe9
void jp_hl(state_t &s) {
    s.pc = s.regs.hl;
}

// 0xea
void ld_nnp_a(state_t &s) {
    write_u8(s, s.operand, s.regs.a);
}

// 0xee
void xor_a_n(state_t &s) {
    _xor_a_n(s, (_reg8_t)s.operand);
}

// 0xe7
void rst_28h(state_t &s) {
    _rst_n(s, 0x28);
}


// 0xf0
void ldh_a_np(state_t &s) {
    s.regs.a = read_u8(s, (_reg16_t)0xff00 + (_reg8_t)s.operand);
}

// 0xf1
void pop_af(state_t &s) { 
    pop_reg16(s, s.regs.af);
    s.regs.f &= 0xf0;
}

// 0xf2
void ld_a_cp(state_t &s) {
    s.regs.a = read_u8(s, (_reg16_t)0xff00 + (_reg8_t)s.regs.c);
}

// 0xf3
void di(state_t &s) {
    _di(s);
}

// 0xf5
void push_af(state_t &s) {
    push_reg16(s, s.regs.af); 
}

// 0xf6
void or_a_n(state_t &s) {
    _or_a_n(s, (_reg8_t)s.operand);
}

// 0xf7
void rst_30h(state_t &s) {
    _rst_n(s, 0x30);
}

// 0xf8
void ldhl_sp_n(state_t &s) {
    _reg16_t sp = s.regs.sp;
    _add_sp_u8(s, (_op8_t)s.operand);
    s.regs.hl = s.regs.sp;
    s.regs.sp = sp;
}

// 0xf9
void ld_sp_hl(state_t &s) {
    s.regs.sp = s.regs.hl;
}

// 0xfa
void ld_a_nnp(state_t &s) {
    s.regs.a = read_u8(s, s.operand);
}

// 0xfb
void ei(state_t &s) {
    _ei(s);
}

// 0xfe
void cp_a_n(state_t &s) {
    _cp_a_n(s, (_reg8_t)s.operand);
}

// 0xf7
void rst_38h(state_t &s) {
    _rst_n(s, 0x38);
}


// =============================================================
// ============ Mapped Unprefixed instructions start ===========
// =============================================================

// 0xcb0
void rlc_b(state_t &s) { s.regs.b = _rlc_n(s, s.regs.b); }
// 0xcb1
void rlc_c(state_t &s) { s.regs.c = _rlc_n(s, s.regs.c); }
// 0xcb2
void rlc_d(state_t &s) { s.regs.d = _rlc_n(s, s.regs.d); }
// 0xcb3
void rlc_e(state_t &s) { s.regs.e = _rlc_n(s, s.regs.e); }
// 0xcb4
void rlc_h(state_t &s) { s.regs.h = _rlc_n(s, s.regs.h); }
// 0xcb5
void rlc_l(state_t &s) { s.regs.l = _rlc_n(s, s.regs.l); }
// 0xcb6
void rlc_hlp(state_t &s) { write_u8(s, s.regs.hl, _rlc_n(s, read_u8(s, s.regs.hl))); }
// 0xcb7
void rlc_a(state_t &s) { s.regs.a = _rlc_n(s, s.regs.a); }
// 0xcb8
void rrc_b(state_t &s) { s.regs.b = _rrc_n(s, s.regs.b); }
// 0xcb9
void rrc_c(state_t &s) { s.regs.c = _rrc_n(s, s.regs.c); }
// 0xcba
void rrc_d(state_t &s) { s.regs.d = _rrc_n(s, s.regs.d); }
// 0xcbb
void rrc_e(state_t &s) { s.regs.e = _rrc_n(s, s.regs.e); }
// 0xcbc
void rrc_h(state_t &s) { s.regs.h = _rrc_n(s, s.regs.h); }
// 0xcbd
void rrc_l(state_t &s) { s.regs.l = _rrc_n(s, s.regs.l); }
// 0xcbe
void rrc_hlp(state_t &s) { write_u8(s, s.regs.hl, _rrc_n(s, read_u8(s, s.regs.hl))); }
// 0xcbf
void rrc_a(state_t &s) { s.regs.a = _rrc_n(s, s.regs.a); }
// 0xcb10
void rl_b(state_t &s) { s.regs.b = _rl_n(s, s.regs.b); }
// 0xcb11
void rl_c(state_t &s) { s.regs.c = _rl_n(s, s.regs.c); }
// 0xcb12
void rl_d(state_t &s) { s.regs.d = _rl_n(s, s.regs.d); }
// 0xcb13
void rl_e(state_t &s) { s.regs.e = _rl_n(s, s.regs.e); }
// 0xcb14
void rl_h(state_t &s) { s.regs.h = _rl_n(s, s.regs.h); }
// 0xcb15
void rl_l(state_t &s) { s.regs.l = _rl_n(s, s.regs.l); }
// 0xcb16
void rl_hlp(state_t &s) { write_u8(s, s.regs.hl, _rl_n(s, read_u8(s, s.regs.hl))); }
// 0xcb17
void rl_a(state_t &s) { s.regs.a = _rl_n(s, s.regs.a); }
// 0xcb18
void rr_b(state_t &s) { s.regs.b = _rr_n(s, s.regs.b); }
// 0xcb19
void rr_c(state_t &s) { s.regs.c = _rr_n(s, s.regs.c); }
// 0xcb1a
void rr_d(state_t &s) { s.regs.d = _rr_n(s, s.regs.d); }
// 0xcb1b
void rr_e(state_t &s) { s.regs.e = _rr_n(s, s.regs.e); }
// 0xcb1c
void rr_h(state_t &s) { s.regs.h = _rr_n(s, s.regs.h); }
// 0xcb1d
void rr_l(state_t &s) { s.regs.l = _rr_n(s, s.regs.l); }
// 0xcb1e
void rr_hlp(state_t &s) { write_u8(s, s.regs.hl, _rr_n(s, read_u8(s, s.regs.hl))); }
// 0xcb1f
void rr_a(state_t &s) { s.regs.a = _rr_n(s, s.regs.a); }
// 0xcb20
void sla_b(state_t &s) { s.regs.b = _sla_n(s, s.regs.b); }
// 0xcb21
void sla_c(state_t &s) { s.regs.c = _sla_n(s, s.regs.c); }
// 0xcb22
void sla_d(state_t &s) { s.regs.d = _sla_n(s, s.regs.d); }
// 0xcb23
void sla_e(state_t &s) { s.regs.e = _sla_n(s, s.regs.e); }
// 0xcb24
void sla_h(state_t &s) { s.regs.h = _sla_n(s, s.regs.h); }
// 0xcb25
void sla_l(state_t &s) { s.regs.l = _sla_n(s, s.regs.l); }
// 0xcb26
void sla_hlp(state_t &s) { write_u8(s, s.regs.hl, _sla_n(s, read_u8(s, s.regs.hl))); }
// 0xcb27
void sla_a(state_t &s) { s.regs.a = _sla_n(s, s.regs.a); }
// 0xcb28
void sra_b(state_t &s) { s.regs.b = _sra_n(s, s.regs.b); }
// 0xcb29
void sra_c(state_t &s) { s.regs.c = _sra_n(s, s.regs.c); }
// 0xcb2a
void sra_d(state_t &s) { s.regs.d = _sra_n(s, s.regs.d); }
// 0xcb2b
void sra_e(state_t &s) { s.regs.e = _sra_n(s, s.regs.e); }
// 0xcb2c
void sra_h(state_t &s) { s.regs.h = _sra_n(s, s.regs.h); }
// 0xcb2d
void sra_l(state_t &s) { s.regs.l = _sra_n(s, s.regs.l); }
// 0xcb2e
void sra_hlp(state_t &s) { write_u8(s, s.regs.hl, _sra_n(s, read_u8(s, s.regs.hl))); }
// 0xcb2f
void sra_a(state_t &s) { s.regs.a = _sra_n(s, s.regs.a); }
// 0xcb30
void swap_b(state_t &s) { s.regs.b = _swap_n(s, s.regs.b); }
// 0xcb31
void swap_c(state_t &s) { s.regs.c = _swap_n(s, s.regs.c); }
// 0xcb32
void swap_d(state_t &s) { s.regs.d = _swap_n(s, s.regs.d); }
// 0xcb33
void swap_e(state_t &s) { s.regs.e = _swap_n(s, s.regs.e); }
// 0xcb34
void swap_h(state_t &s) { s.regs.h = _swap_n(s, s.regs.h); }
// 0xcb35
void swap_l(state_t &s) { s.regs.l = _swap_n(s, s.regs.l); }
// 0xcb36
void swap_hlp(state_t &s) { write_u8(s, s.regs.hl, _swap_n(s, read_u8(s, s.regs.hl))); }
// 0xcb37
void swap_a(state_t &s) { s.regs.a = _swap_n(s, s.regs.a); }
// 0xcb38
void srl_b(state_t &s) { s.regs.b = _srl_n(s, s.regs.b); }
// 0xcb39
void srl_c(state_t &s) { s.regs.c = _srl_n(s, s.regs.c); }
// 0xcb3a
void srl_d(state_t &s) { s.regs.d = _srl_n(s, s.regs.d); }
// 0xcb3b
void srl_e(state_t &s) { s.regs.e = _srl_n(s, s.regs.e); }
// 0xcb3c
void srl_h(state_t &s) { s.regs.h = _srl_n(s, s.regs.h); }
// 0xcb3d
void srl_l(state_t &s) { s.regs.l = _srl_n(s, s.regs.l); }
// 0xcb3e
void srl_hlp(state_t &s) { write_u8(s, s.regs.hl, _srl_n(s, read_u8(s, s.regs.hl))); }
// 0xcb3f
void srl_a(state_t &s) { s.regs.a = _srl_n(s, s.regs.a); }
// 0xcb40
void bit_0_b(state_t &s) { _bit_b_r(s, 0, s.regs.b); }
// 0xcb41
void bit_0_c(state_t &s) { _bit_b_r(s, 0, s.regs.c); }
// 0xcb42
void bit_0_d(state_t &s) { _bit_b_r(s, 0, s.regs.d); }
// 0xcb43
void bit_0_e(state_t &s) { _bit_b_r(s, 0, s.regs.e); }
// 0xcb44
void bit_0_h(state_t &s) { _bit_b_r(s, 0, s.regs.h); }
// 0xcb45
void bit_0_l(state_t &s) { _bit_b_r(s, 0, s.regs.l); }
// 0xcb46
void bit_0_hlp(state_t &s) { _bit_b_r(s, 0, read_u8(s, s.regs.hl)); }
// 0xcb47
void bit_0_a(state_t &s) { _bit_b_r(s, 0, s.regs.a); }
// 0xcb48
void bit_1_b(state_t &s) { _bit_b_r(s, 1, s.regs.b); }
// 0xcb49
void bit_1_c(state_t &s) { _bit_b_r(s, 1, s.regs.c); }
// 0xcb4a
void bit_1_d(state_t &s) { _bit_b_r(s, 1, s.regs.d); }
// 0xcb4b
void bit_1_e(state_t &s) { _bit_b_r(s, 1, s.regs.e); }
// 0xcb4c
void bit_1_h(state_t &s) { _bit_b_r(s, 1, s.regs.h); }
// 0xcb4d
void bit_1_l(state_t &s) { _bit_b_r(s, 1, s.regs.l); }
// 0xcb4e
void bit_1_hlp(state_t &s) { _bit_b_r(s, 1, read_u8(s, s.regs.hl)); }
// 0xcb4f
void bit_1_a(state_t &s) { _bit_b_r(s, 1, s.regs.a); }
// 0xcb50
void bit_2_b(state_t &s) { _bit_b_r(s, 2, s.regs.b); }
// 0xcb51
void bit_2_c(state_t &s) { _bit_b_r(s, 2, s.regs.c); }
// 0xcb52
void bit_2_d(state_t &s) { _bit_b_r(s, 2, s.regs.d); }
// 0xcb53
void bit_2_e(state_t &s) { _bit_b_r(s, 2, s.regs.e); }
// 0xcb54
void bit_2_h(state_t &s) { _bit_b_r(s, 2, s.regs.h); }
// 0xcb55
void bit_2_l(state_t &s) { _bit_b_r(s, 2, s.regs.l); }
// 0xcb56
void bit_2_hlp(state_t &s) { _bit_b_r(s, 2, read_u8(s, s.regs.hl)); }
// 0xcb57
void bit_2_a(state_t &s) { _bit_b_r(s, 2, s.regs.a); }
// 0xcb58
void bit_3_b(state_t &s) { _bit_b_r(s, 3, s.regs.b); }
// 0xcb59
void bit_3_c(state_t &s) { _bit_b_r(s, 3, s.regs.c); }
// 0xcb5a
void bit_3_d(state_t &s) { _bit_b_r(s, 3, s.regs.d); }
// 0xcb5b
void bit_3_e(state_t &s) { _bit_b_r(s, 3, s.regs.e); }
// 0xcb5c
void bit_3_h(state_t &s) { _bit_b_r(s, 3, s.regs.h); }
// 0xcb5d
void bit_3_l(state_t &s) { _bit_b_r(s, 3, s.regs.l); }
// 0xcb5e
void bit_3_hlp(state_t &s) { _bit_b_r(s, 3, read_u8(s, s.regs.hl)); }
// 0xcb5f
void bit_3_a(state_t &s) { _bit_b_r(s, 3, s.regs.a); }
// 0xcb60
void bit_4_b(state_t &s) { _bit_b_r(s, 4, s.regs.b); }
// 0xcb61
void bit_4_c(state_t &s) { _bit_b_r(s, 4, s.regs.c); }
// 0xcb62
void bit_4_d(state_t &s) { _bit_b_r(s, 4, s.regs.d); }
// 0xcb63
void bit_4_e(state_t &s) { _bit_b_r(s, 4, s.regs.e); }
// 0xcb64
void bit_4_h(state_t &s) { _bit_b_r(s, 4, s.regs.h); }
// 0xcb65
void bit_4_l(state_t &s) { _bit_b_r(s, 4, s.regs.l); }
// 0xcb66
void bit_4_hlp(state_t &s) { _bit_b_r(s, 4, read_u8(s, s.regs.hl)); }
// 0xcb67
void bit_4_a(state_t &s) { _bit_b_r(s, 4, s.regs.a); }
// 0xcb68
void bit_5_b(state_t &s) { _bit_b_r(s, 5, s.regs.b); }
// 0xcb69
void bit_5_c(state_t &s) { _bit_b_r(s, 5, s.regs.c); }
// 0xcb6a
void bit_5_d(state_t &s) { _bit_b_r(s, 5, s.regs.d); }
// 0xcb6b
void bit_5_e(state_t &s) { _bit_b_r(s, 5, s.regs.e); }
// 0xcb6c
void bit_5_h(state_t &s) { _bit_b_r(s, 5, s.regs.h); }
// 0xcb6d
void bit_5_l(state_t &s) { _bit_b_r(s, 5, s.regs.l); }
// 0xcb6e
void bit_5_hlp(state_t &s) { _bit_b_r(s, 5, read_u8(s, s.regs.hl)); }
// 0xcb6f
void bit_5_a(state_t &s) { _bit_b_r(s, 5, s.regs.a); }
// 0xcb70
void bit_6_b(state_t &s) { _bit_b_r(s, 6, s.regs.b); }
// 0xcb71
void bit_6_c(state_t &s) { _bit_b_r(s, 6, s.regs.c); }
// 0xcb72
void bit_6_d(state_t &s) { _bit_b_r(s, 6, s.regs.d); }
// 0xcb73
void bit_6_e(state_t &s) { _bit_b_r(s, 6, s.regs.e); }
// 0xcb74
void bit_6_h(state_t &s) { _bit_b_r(s, 6, s.regs.h); }
// 0xcb75
void bit_6_l(state_t &s) { _bit_b_r(s, 6, s.regs.l); }
// 0xcb76
void bit_6_hlp(state_t &s) { _bit_b_r(s, 6, read_u8(s, s.regs.hl)); }
// 0xcb77
void bit_6_a(state_t &s) { _bit_b_r(s, 6, s.regs.a); }
// 0xcb78
void bit_7_b(state_t &s) { _bit_b_r(s, 7, s.regs.b); }
// 0xcb79
void bit_7_c(state_t &s) { _bit_b_r(s, 7, s.regs.c); }
// 0xcb7a
void bit_7_d(state_t &s) { _bit_b_r(s, 7, s.regs.d); }
// 0xcb7b
void bit_7_e(state_t &s) { _bit_b_r(s, 7, s.regs.e); }
// 0xcb7c
void bit_7_h(state_t &s) { _bit_b_r(s, 7, s.regs.h); }
// 0xcb7d
void bit_7_l(state_t &s) { _bit_b_r(s, 7, s.regs.l); }
// 0xcb7e
void bit_7_hlp(state_t &s) { _bit_b_r(s, 7, read_u8(s, s.regs.hl)); }
// 0xcb7f
void bit_7_a(state_t &s) { _bit_b_r(s, 7, s.regs.a); }
// 0xcb80
void set_0_b(state_t &s) { s.regs.b = _set_b_r(s, 0, s.regs.b); }
// 0xcb81
void set_0_c(state_t &s) { s.regs.c = _set_b_r(s, 0, s.regs.c); }
// 0xcb82
void set_0_d(state_t &s) { s.regs.d = _set_b_r(s, 0, s.regs.d); }
// 0xcb83
void set_0_e(state_t &s) { s.regs.e = _set_b_r(s, 0, s.regs.e); }
// 0xcb84
void set_0_h(state_t &s) { s.regs.h = _set_b_r(s, 0, s.regs.h); }
// 0xcb85
void set_0_l(state_t &s) { s.regs.l = _set_b_r(s, 0, s.regs.l); }
// 0xcb86
void set_0_hlp(state_t &s) { write_u8(s, s.regs.hl, _set_b_r(s, 0, read_u8(s, s.regs.hl))); }
// 0xcb87
void set_0_a(state_t &s) { s.regs.a = _set_b_r(s, 0, s.regs.a); }
// 0xcb88
void set_1_b(state_t &s) { s.regs.b = _set_b_r(s, 1, s.regs.b); }
// 0xcb89
void set_1_c(state_t &s) { s.regs.c = _set_b_r(s, 1, s.regs.c); }
// 0xcb8a
void set_1_d(state_t &s) { s.regs.d = _set_b_r(s, 1, s.regs.d); }
// 0xcb8b
void set_1_e(state_t &s) { s.regs.e = _set_b_r(s, 1, s.regs.e); }
// 0xcb8c
void set_1_h(state_t &s) { s.regs.h = _set_b_r(s, 1, s.regs.h); }
// 0xcb8d
void set_1_l(state_t &s) { s.regs.l = _set_b_r(s, 1, s.regs.l); }
// 0xcb8e
void set_1_hlp(state_t &s) { write_u8(s, s.regs.hl, _set_b_r(s, 1, read_u8(s, s.regs.hl))); }
// 0xcb8f
void set_1_a(state_t &s) { s.regs.a = _set_b_r(s, 1, s.regs.a); }
// 0xcb90
void set_2_b(state_t &s) { s.regs.b = _set_b_r(s, 2, s.regs.b); }
// 0xcb91
void set_2_c(state_t &s) { s.regs.c = _set_b_r(s, 2, s.regs.c); }
// 0xcb92
void set_2_d(state_t &s) { s.regs.d = _set_b_r(s, 2, s.regs.d); }
// 0xcb93
void set_2_e(state_t &s) { s.regs.e = _set_b_r(s, 2, s.regs.e); }
// 0xcb94
void set_2_h(state_t &s) { s.regs.h = _set_b_r(s, 2, s.regs.h); }
// 0xcb95
void set_2_l(state_t &s) { s.regs.l = _set_b_r(s, 2, s.regs.l); }
// 0xcb96
void set_2_hlp(state_t &s) { write_u8(s, s.regs.hl, _set_b_r(s, 2, read_u8(s, s.regs.hl))); }
// 0xcb97
void set_2_a(state_t &s) { s.regs.a = _set_b_r(s, 2, s.regs.a); }
// 0xcb98
void set_3_b(state_t &s) { s.regs.b = _set_b_r(s, 3, s.regs.b); }
// 0xcb99
void set_3_c(state_t &s) { s.regs.c = _set_b_r(s, 3, s.regs.c); }
// 0xcb9a
void set_3_d(state_t &s) { s.regs.d = _set_b_r(s, 3, s.regs.d); }
// 0xcb9b
void set_3_e(state_t &s) { s.regs.e = _set_b_r(s, 3, s.regs.e); }
// 0xcb9c
void set_3_h(state_t &s) { s.regs.h = _set_b_r(s, 3, s.regs.h); }
// 0xcb9d
void set_3_l(state_t &s) { s.regs.l = _set_b_r(s, 3, s.regs.l); }
// 0xcb9e
void set_3_hlp(state_t &s) { write_u8(s, s.regs.hl, _set_b_r(s, 3, read_u8(s, s.regs.hl))); }
// 0xcb9f
void set_3_a(state_t &s) { s.regs.a = _set_b_r(s, 3, s.regs.a); }
// 0xcba0
void set_4_b(state_t &s) { s.regs.b = _set_b_r(s, 4, s.regs.b); }
// 0xcba1
void set_4_c(state_t &s) { s.regs.c = _set_b_r(s, 4, s.regs.c); }
// 0xcba2
void set_4_d(state_t &s) { s.regs.d = _set_b_r(s, 4, s.regs.d); }
// 0xcba3
void set_4_e(state_t &s) { s.regs.e = _set_b_r(s, 4, s.regs.e); }
// 0xcba4
void set_4_h(state_t &s) { s.regs.h = _set_b_r(s, 4, s.regs.h); }
// 0xcba5
void set_4_l(state_t &s) { s.regs.l = _set_b_r(s, 4, s.regs.l); }
// 0xcba6
void set_4_hlp(state_t &s) { write_u8(s, s.regs.hl, _set_b_r(s, 4, read_u8(s, s.regs.hl))); }
// 0xcba7
void set_4_a(state_t &s) { s.regs.a = _set_b_r(s, 4, s.regs.a); }
// 0xcba8
void set_5_b(state_t &s) { s.regs.b = _set_b_r(s, 5, s.regs.b); }
// 0xcba9
void set_5_c(state_t &s) { s.regs.c = _set_b_r(s, 5, s.regs.c); }
// 0xcbaa
void set_5_d(state_t &s) { s.regs.d = _set_b_r(s, 5, s.regs.d); }
// 0xcbab
void set_5_e(state_t &s) { s.regs.e = _set_b_r(s, 5, s.regs.e); }
// 0xcbac
void set_5_h(state_t &s) { s.regs.h = _set_b_r(s, 5, s.regs.h); }
// 0xcbad
void set_5_l(state_t &s) { s.regs.l = _set_b_r(s, 5, s.regs.l); }
// 0xcbae
void set_5_hlp(state_t &s) { write_u8(s, s.regs.hl, _set_b_r(s, 5, read_u8(s, s.regs.hl))); }
// 0xcbaf
void set_5_a(state_t &s) { s.regs.a = _set_b_r(s, 5, s.regs.a); }
// 0xcbb0
void set_6_b(state_t &s) { s.regs.b = _set_b_r(s, 6, s.regs.b); }
// 0xcbb1
void set_6_c(state_t &s) { s.regs.c = _set_b_r(s, 6, s.regs.c); }
// 0xcbb2
void set_6_d(state_t &s) { s.regs.d = _set_b_r(s, 6, s.regs.d); }
// 0xcbb3
void set_6_e(state_t &s) { s.regs.e = _set_b_r(s, 6, s.regs.e); }
// 0xcbb4
void set_6_h(state_t &s) { s.regs.h = _set_b_r(s, 6, s.regs.h); }
// 0xcbb5
void set_6_l(state_t &s) { s.regs.l = _set_b_r(s, 6, s.regs.l); }
// 0xcbb6
void set_6_hlp(state_t &s) { write_u8(s, s.regs.hl, _set_b_r(s, 6, read_u8(s, s.regs.hl))); }
// 0xcbb7
void set_6_a(state_t &s) { s.regs.a = _set_b_r(s, 6, s.regs.a); }
// 0xcbb8
void set_7_b(state_t &s) { s.regs.b = _set_b_r(s, 7, s.regs.b); }
// 0xcbb9
void set_7_c(state_t &s) { s.regs.c = _set_b_r(s, 7, s.regs.c); }
// 0xcbba
void set_7_d(state_t &s) { s.regs.d = _set_b_r(s, 7, s.regs.d); }
// 0xcbbb
void set_7_e(state_t &s) { s.regs.e = _set_b_r(s, 7, s.regs.e); }
// 0xcbbc
void set_7_h(state_t &s) { s.regs.h = _set_b_r(s, 7, s.regs.h); }
// 0xcbbd
void set_7_l(state_t &s) { s.regs.l = _set_b_r(s, 7, s.regs.l); }
// 0xcbbe
void set_7_hlp(state_t &s) { write_u8(s, s.regs.hl, _set_b_r(s, 7, read_u8(s, s.regs.hl))); }
// 0xcbbf
void set_7_a(state_t &s) { s.regs.a = _set_b_r(s, 7, s.regs.a); }
// 0xcbc0
void res_0_b(state_t &s) { s.regs.b = _res_b_r(s, 0, s.regs.b); }
// 0xcbc1
void res_0_c(state_t &s) { s.regs.c = _res_b_r(s, 0, s.regs.c); }
// 0xcbc2
void res_0_d(state_t &s) { s.regs.d = _res_b_r(s, 0, s.regs.d); }
// 0xcbc3
void res_0_e(state_t &s) { s.regs.e = _res_b_r(s, 0, s.regs.e); }
// 0xcbc4
void res_0_h(state_t &s) { s.regs.h = _res_b_r(s, 0, s.regs.h); }
// 0xcbc5
void res_0_l(state_t &s) { s.regs.l = _res_b_r(s, 0, s.regs.l); }
// 0xcbc6
void res_0_hlp(state_t &s) { write_u8(s, s.regs.hl, _res_b_r(s, 0, read_u8(s, s.regs.hl))); }
// 0xcbc7
void res_0_a(state_t &s) { s.regs.a = _res_b_r(s, 0, s.regs.a); }
// 0xcbc8
void res_1_b(state_t &s) { s.regs.b = _res_b_r(s, 1, s.regs.b); }
// 0xcbc9
void res_1_c(state_t &s) { s.regs.c = _res_b_r(s, 1, s.regs.c); }
// 0xcbca
void res_1_d(state_t &s) { s.regs.d = _res_b_r(s, 1, s.regs.d); }
// 0xcbcb
void res_1_e(state_t &s) { s.regs.e = _res_b_r(s, 1, s.regs.e); }
// 0xcbcc
void res_1_h(state_t &s) { s.regs.h = _res_b_r(s, 1, s.regs.h); }
// 0xcbcd
void res_1_l(state_t &s) { s.regs.l = _res_b_r(s, 1, s.regs.l); }
// 0xcbce
void res_1_hlp(state_t &s) { write_u8(s, s.regs.hl, _res_b_r(s, 1, read_u8(s, s.regs.hl))); }
// 0xcbcf
void res_1_a(state_t &s) { s.regs.a = _res_b_r(s, 1, s.regs.a); }
// 0xcbd0
void res_2_b(state_t &s) { s.regs.b = _res_b_r(s, 2, s.regs.b); }
// 0xcbd1
void res_2_c(state_t &s) { s.regs.c = _res_b_r(s, 2, s.regs.c); }
// 0xcbd2
void res_2_d(state_t &s) { s.regs.d = _res_b_r(s, 2, s.regs.d); }
// 0xcbd3
void res_2_e(state_t &s) { s.regs.e = _res_b_r(s, 2, s.regs.e); }
// 0xcbd4
void res_2_h(state_t &s) { s.regs.h = _res_b_r(s, 2, s.regs.h); }
// 0xcbd5
void res_2_l(state_t &s) { s.regs.l = _res_b_r(s, 2, s.regs.l); }
// 0xcbd6
void res_2_hlp(state_t &s) { write_u8(s, s.regs.hl, _res_b_r(s, 2, read_u8(s, s.regs.hl))); }
// 0xcbd7
void res_2_a(state_t &s) { s.regs.a = _res_b_r(s, 2, s.regs.a); }
// 0xcbd8
void res_3_b(state_t &s) { s.regs.b = _res_b_r(s, 3, s.regs.b); }
// 0xcbd9
void res_3_c(state_t &s) { s.regs.c = _res_b_r(s, 3, s.regs.c); }
// 0xcbda
void res_3_d(state_t &s) { s.regs.d = _res_b_r(s, 3, s.regs.d); }
// 0xcbdb
void res_3_e(state_t &s) { s.regs.e = _res_b_r(s, 3, s.regs.e); }
// 0xcbdc
void res_3_h(state_t &s) { s.regs.h = _res_b_r(s, 3, s.regs.h); }
// 0xcbdd
void res_3_l(state_t &s) { s.regs.l = _res_b_r(s, 3, s.regs.l); }
// 0xcbde
void res_3_hlp(state_t &s) { write_u8(s, s.regs.hl, _res_b_r(s, 3, read_u8(s, s.regs.hl))); }
// 0xcbdf
void res_3_a(state_t &s) { s.regs.a = _res_b_r(s, 3, s.regs.a); }
// 0xcbe0
void res_4_b(state_t &s) { s.regs.b = _res_b_r(s, 4, s.regs.b); }
// 0xcbe1
void res_4_c(state_t &s) { s.regs.c = _res_b_r(s, 4, s.regs.c); }
// 0xcbe2
void res_4_d(state_t &s) { s.regs.d = _res_b_r(s, 4, s.regs.d); }
// 0xcbe3
void res_4_e(state_t &s) { s.regs.e = _res_b_r(s, 4, s.regs.e); }
// 0xcbe4
void res_4_h(state_t &s) { s.regs.h = _res_b_r(s, 4, s.regs.h); }
// 0xcbe5
void res_4_l(state_t &s) { s.regs.l = _res_b_r(s, 4, s.regs.l); }
// 0xcbe6
void res_4_hlp(state_t &s) { write_u8(s, s.regs.hl, _res_b_r(s, 4, read_u8(s, s.regs.hl))); }
// 0xcbe7
void res_4_a(state_t &s) { s.regs.a = _res_b_r(s, 4, s.regs.a); }
// 0xcbe8
void res_5_b(state_t &s) { s.regs.b = _res_b_r(s, 5, s.regs.b); }
// 0xcbe9
void res_5_c(state_t &s) { s.regs.c = _res_b_r(s, 5, s.regs.c); }
// 0xcbea
void res_5_d(state_t &s) { s.regs.d = _res_b_r(s, 5, s.regs.d); }
// 0xcbeb
void res_5_e(state_t &s) { s.regs.e = _res_b_r(s, 5, s.regs.e); }
// 0xcbec
void res_5_h(state_t &s) { s.regs.h = _res_b_r(s, 5, s.regs.h); }
// 0xcbed
void res_5_l(state_t &s) { s.regs.l = _res_b_r(s, 5, s.regs.l); }
// 0xcbee
void res_5_hlp(state_t &s) { write_u8(s, s.regs.hl, _res_b_r(s, 5, read_u8(s, s.regs.hl))); }
// 0xcbef
void res_5_a(state_t &s) { s.regs.a = _res_b_r(s, 5, s.regs.a); }
// 0xcbf0
void res_6_b(state_t &s) { s.regs.b = _res_b_r(s, 6, s.regs.b); }
// 0xcbf1
void res_6_c(state_t &s) { s.regs.c = _res_b_r(s, 6, s.regs.c); }
// 0xcbf2
void res_6_d(state_t &s) { s.regs.d = _res_b_r(s, 6, s.regs.d); }
// 0xcbf3
void res_6_e(state_t &s) { s.regs.e = _res_b_r(s, 6, s.regs.e); }
// 0xcbf4
void res_6_h(state_t &s) { s.regs.h = _res_b_r(s, 6, s.regs.h); }
// 0xcbf5
void res_6_l(state_t &s) { s.regs.l = _res_b_r(s, 6, s.regs.l); }
// 0xcbf6
void res_6_hlp(state_t &s) { write_u8(s, s.regs.hl, _res_b_r(s, 6, read_u8(s, s.regs.hl))); }
// 0xcbf7
void res_6_a(state_t &s) { s.regs.a = _res_b_r(s, 6, s.regs.a); }
// 0xcbf8
void res_7_b(state_t &s) { s.regs.b = _res_b_r(s, 7, s.regs.b); }
// 0xcbf9
void res_7_c(state_t &s) { s.regs.c = _res_b_r(s, 7, s.regs.c); }
// 0xcbfa
void res_7_d(state_t &s) { s.regs.d = _res_b_r(s, 7, s.regs.d); }
// 0xcbfb
void res_7_e(state_t &s) { s.regs.e = _res_b_r(s, 7, s.regs.e); }
// 0xcbfc
void res_7_h(state_t &s) { s.regs.h = _res_b_r(s, 7, s.regs.h); }
// 0xcbfd
void res_7_l(state_t &s) { s.regs.l = _res_b_r(s, 7, s.regs.l); }
// 0xcbfe
void res_7_hlp(state_t &s) { write_u8(s, s.regs.hl, _res_b_r(s, 7, read_u8(s, s.regs.hl))); }
// 0xcbff
void res_7_a(state_t &s) { s.regs.a = _res_b_r(s, 7, s.regs.a); }

const instruction_t instructions[] = {
    {"NOP", 1, 4, 0, nop},
    {"LD BC,0x%04X", 3, 12, 2, ld_bc_nn},
    {"LD (BC),A", 1, 8, 0, ld_bcp_a},
    {"INC BC", 1, 8, 0, inc_bc},
    {"INC B", 1, 4, 0, inc_b},
    {"DEC B", 1, 4, 0, dec_b},
    {"LD B,u8", 2, 8, 1, ld_b_n},
    {"RLCA", 1, 4, 0, rlca},
    {"LD (0x%04X),SP", 3, 20, 2, ld_nnp_sp},
    {"ADD HL,BC", 1, 8, 0, add_hl_bc},
    {"LD A,(BC)", 1, 8, 0, ld_a_bcp},
    {"DEC BC", 1, 8, 0, dec_bc},
    {"INC C", 1, 4, 0, inc_c},
    {"DEC C", 1, 4, 0, dec_c},
    {"LD C,u8", 2, 8, 1, ld_c_n},
    {"RRCA", 1, 4, 0, rrca},
    {"STOP", 1, 4, 0, stop},
    {"LD DE,0x%04X", 3, 12, 2, ld_de_nn},
    {"LD (DE),A", 1, 8, 0, ld_dep_a},
    {"INC DE", 1, 8, 0, inc_de},
    {"INC D", 1, 4, 0, inc_d},
    {"DEC D", 1, 4, 0, dec_d},
    {"LD D,u8", 2, 8, 1, ld_d_n},
    {"RLA", 1, 4, 0, rla},
    {"JR i8", 2, 12, 1, jr_n},
    {"ADD HL,DE", 1, 8, 0, add_hl_de},
    {"LD A,(DE)", 1, 8, 0, ld_a_dep},
    {"DEC DE", 1, 8, 0, dec_de},
    {"INC E", 1, 4, 0, inc_e},
    {"DEC E", 1, 4, 0, dec_e},
    {"LD E,u8", 2, 8, 1, ld_e_n},
    {"RRA", 1, 4, 0, rra},
    {"JR NZ,i8", 2, 12, 1, jr_nz_n},
    {"LD HL,0x%04X", 3, 12, 2, ld_hl_nn},
    {"LD (HL+),A", 1, 8, 0, ldi_hlp_a},
    {"INC HL", 1, 8, 0, inc_hl},
    {"INC H", 1, 4, 0, inc_h},
    {"DEC H", 1, 4, 0, dec_h},
    {"LD H,u8", 2, 8, 1, ld_h_n},
    {"DAA", 1, 4, 0, daa},
    {"JR Z,i8", 2, 12, 1, jr_z_n},
    {"ADD HL,HL", 1, 8, 0, add_hl_hl},
    {"LD A,(HL+)", 1, 8, 0, ldi_a_hlp},
    {"DEC HL", 1, 8, 0, dec_hl},
    {"INC L", 1, 4, 0, inc_l},
    {"DEC L", 1, 4, 0, dec_l},
    {"LD L,u8", 2, 8, 1, ld_l_n},
    {"CPL", 1, 4, 0, cpl},
    {"JR NC,i8", 2, 12, 1, jr_nc_n},
    {"LD SP,0x%04X", 3, 12, 2, ld_sp_nn},
    {"LD (HL-),A", 1, 8, 0, ldd_hlp_a},
    {"INC SP", 1, 8, 0, inc_sp},
    {"INC (HL)", 1, 12, 0, inc_hlp},
    {"DEC (HL)", 1, 12, 0, dec_hlp},
    {"LD (HL),u8", 2, 12, 1, ld_hlp_n},
    {"SCF", 1, 4, 0, scf},
    {"JR C,i8", 2, 12, 1, jr_c_n},
    {"ADD HL,SP", 1, 8, 0, add_hl_sp},
    {"LD A,(HL-)", 1, 8, 0, ldd_a_hlp},
    {"DEC SP", 1, 8, 0, dec_sp},
    {"INC A", 1, 4, 0, inc_a},
    {"DEC A", 1, 4, 0, dec_a},
    {"LD A,u8", 2, 8, 1, ld_a_n},
    {"CCF", 1, 4, 0, ccf},
    {"LD B,B", 1, 4, 0, ld_b_b},
    {"LD B,C", 1, 4, 0, ld_b_c},
    {"LD B,D", 1, 4, 0, ld_b_d},
    {"LD B,E", 1, 4, 0, ld_b_e},
    {"LD B,H", 1, 4, 0, ld_b_h},
    {"LD B,L", 1, 4, 0, ld_b_l},
    {"LD B,(HL)", 1, 8, 0, ld_b_hlp},
    {"LD B,A", 1, 4, 0, ld_b_a},
    {"LD C,B", 1, 4, 0, ld_c_b},
    {"LD C,C", 1, 4, 0, ld_c_c},
    {"LD C,D", 1, 4, 0, ld_c_d},
    {"LD C,E", 1, 4, 0, ld_c_e},
    {"LD C,H", 1, 4, 0, ld_c_h},
    {"LD C,L", 1, 4, 0, ld_c_l},
    {"LD C,(HL)", 1, 8, 0, ld_c_hlp},
    {"LD C,A", 1, 4, 0, ld_c_a},
    {"LD D,B", 1, 4, 0, ld_d_b},
    {"LD D,C", 1, 4, 0, ld_d_c},
    {"LD D,D", 1, 4, 0, ld_d_d},
    {"LD D,E", 1, 4, 0, ld_d_e},
    {"LD D,H", 1, 4, 0, ld_d_h},
    {"LD D,L", 1, 4, 0, ld_d_l},
    {"LD D,(HL)", 1, 8, 0, ld_d_hlp},
    {"LD D,A", 1, 4, 0, ld_d_a},
    {"LD E,B", 1, 4, 0, ld_e_b},
    {"LD E,C", 1, 4, 0, ld_e_c},
    {"LD E,D", 1, 4, 0, ld_e_d},
    {"LD E,E", 1, 4, 0, ld_e_e},
    {"LD E,H", 1, 4, 0, ld_e_h},
    {"LD E,L", 1, 4, 0, ld_e_l},
    {"LD E,(HL)", 1, 8, 0, ld_e_hlp},
    {"LD E,A", 1, 4, 0, ld_e_a},
    {"LD H,B", 1, 4, 0, ld_h_b},
    {"LD H,C", 1, 4, 0, ld_h_c},
    {"LD H,D", 1, 4, 0, ld_h_d},
    {"LD H,E", 1, 4, 0, ld_h_e},
    {"LD H,H", 1, 4, 0, ld_h_h},
    {"LD H,L", 1, 4, 0, ld_h_l},
    {"LD H,(HL)", 1, 8, 0, ld_h_hlp},
    {"LD H,A", 1, 4, 0, ld_h_a},
    {"LD L,B", 1, 4, 0, ld_l_b},
    {"LD L,C", 1, 4, 0, ld_l_c},
    {"LD L,D", 1, 4, 0, ld_l_d},
    {"LD L,E", 1, 4, 0, ld_l_e},
    {"LD L,H", 1, 4, 0, ld_l_h},
    {"LD L,L", 1, 4, 0, ld_l_l},
    {"LD L,(HL)", 1, 8, 0, ld_l_hlp},
    {"LD L,A", 1, 4, 0, ld_l_a},
    {"LD (HL),B", 1, 8, 0, ld_hlp_b},
    {"LD (HL),C", 1, 8, 0, ld_hlp_c},
    {"LD (HL),D", 1, 8, 0, ld_hlp_d},
    {"LD (HL),E", 1, 8, 0, ld_hlp_e},
    {"LD (HL),H", 1, 8, 0, ld_hlp_h},
    {"LD (HL),L", 1, 8, 0, ld_hlp_l},
    {"HALT", 1, 4, 0, halt},
    {"LD (HL),A", 1, 8, 0, ld_hlp_a},
    {"LD A,B", 1, 4, 0, ld_a_b},
    {"LD A,C", 1, 4, 0, ld_a_c},
    {"LD A,D", 1, 4, 0, ld_a_d},
    {"LD A,E", 1, 4, 0, ld_a_e},
    {"LD A,H", 1, 4, 0, ld_a_h},
    {"LD A,L", 1, 4, 0, ld_a_l},
    {"LD A,(HL)", 1, 8, 0, ld_a_hlp},
    {"LD A,A", 1, 4, 0, ld_a_a},
    {"ADD A,B", 1, 4, 0, add_a_b},
    {"ADD A,C", 1, 4, 0, add_a_c},
    {"ADD A,D", 1, 4, 0, add_a_d},
    {"ADD A,E", 1, 4, 0, add_a_e},
    {"ADD A,H", 1, 4, 0, add_a_h},
    {"ADD A,L", 1, 4, 0, add_a_l},
    {"ADD A,(HL)", 1, 8, 0, add_a_hlp},
    {"ADD A,A", 1, 4, 0, add_a_a},
    {"ADC A,B", 1, 4, 0, adc_a_b},
    {"ADC A,C", 1, 4, 0, adc_a_c},
    {"ADC A,D", 1, 4, 0, adc_a_d},
    {"ADC A,E", 1, 4, 0, adc_a_e},
    {"ADC A,H", 1, 4, 0, adc_a_h},
    {"ADC A,L", 1, 4, 0, adc_a_l},
    {"ADC A,(HL)", 1, 8, 0, adc_a_hlp},
    {"ADC A,A", 1, 4, 0, adc_a_a},
    {"SUB A,B", 1, 4, 0, sub_a_b},
    {"SUB A,C", 1, 4, 0, sub_a_c},
    {"SUB A,D", 1, 4, 0, sub_a_d},
    {"SUB A,E", 1, 4, 0, sub_a_e},
    {"SUB A,H", 1, 4, 0, sub_a_h},
    {"SUB A,L", 1, 4, 0, sub_a_l},
    {"SUB A,(HL)", 1, 8, 0, sub_a_hlp},
    {"SUB A,A", 1, 4, 0, sub_a_a},
    {"SBC A,B", 1, 4, 0, sbc_a_b},
    {"SBC A,C", 1, 4, 0, sbc_a_c},
    {"SBC A,D", 1, 4, 0, sbc_a_d},
    {"SBC A,E", 1, 4, 0, sbc_a_e},
    {"SBC A,H", 1, 4, 0, sbc_a_h},
    {"SBC A,L", 1, 4, 0, sbc_a_l},
    {"SBC A,(HL)", 1, 8, 0, sbc_a_hlp},
    {"SBC A,A", 1, 4, 0, sbc_a_a},
    {"AND A,B", 1, 4, 0, and_a_b},
    {"AND A,C", 1, 4, 0, and_a_c},
    {"AND A,D", 1, 4, 0, and_a_d},
    {"AND A,E", 1, 4, 0, and_a_e},
    {"AND A,H", 1, 4, 0, and_a_h},
    {"AND A,L", 1, 4, 0, and_a_l},
    {"AND A,(HL)", 1, 8, 0, and_a_hlp},
    {"AND A,A", 1, 4, 0, and_a_a},
    {"XOR A,B", 1, 4, 0, xor_a_b},
    {"XOR A,C", 1, 4, 0, xor_a_c},
    {"XOR A,D", 1, 4, 0, xor_a_d},
    {"XOR A,E", 1, 4, 0, xor_a_e},
    {"XOR A,H", 1, 4, 0, xor_a_h},
    {"XOR A,L", 1, 4, 0, xor_a_l},
    {"XOR A,(HL)", 1, 8, 0, xor_a_hlp},
    {"XOR A,A", 1, 4, 0, xor_a_a},
    {"OR A,B", 1, 4, 0, or_a_b},
    {"OR A,C", 1, 4, 0, or_a_c},
    {"OR A,D", 1, 4, 0, or_a_d},
    {"OR A,E", 1, 4, 0, or_a_e},
    {"OR A,H", 1, 4, 0, or_a_h},
    {"OR A,L", 1, 4, 0, or_a_l},
    {"OR A,(HL)", 1, 8, 0, or_a_hlp},
    {"OR A,A", 1, 4, 0, or_a_a},
    {"CP A,B", 1, 4, 0, cp_a_b},
    {"CP A,C", 1, 4, 0, cp_a_c},
    {"CP A,D", 1, 4, 0, cp_a_d},
    {"CP A,E", 1, 4, 0, cp_a_e},
    {"CP A,H", 1, 4, 0, cp_a_h},
    {"CP A,L", 1, 4, 0, cp_a_l},
    {"CP A,(HL)", 1, 8, 0, cp_a_hlp},
    {"CP A,A", 1, 4, 0, cp_a_a},
    {"RET NZ", 1, 20, 0, ret_nz},
    {"POP BC", 1, 12, 0, pop_bc},
    {"JP NZ,0x%04X", 3, 16, 2, jp_nz_nn},
    {"JP 0x%04X", 3, 16, 2, jp_nn},
    {"CALL NZ,0x%04X", 3, 24, 2, call_nz_nn},
    {"PUSH BC", 1, 16, 0, push_bc},
    {"ADD A,u8", 2, 8, 1, add_a_n},
    {"RST 00h", 1, 16, 0, rst_00h},
    {"RET Z", 1, 20, 0, ret_z},
    {"RET", 1, 16, 0, ret},
    {"JP Z,0x%04X", 3, 16, 2, jp_z_nn},
    {"PREFIX CB", 1, 4, 0, prefix_cb},
    {"CALL Z,0x%04X", 3, 24, 2, call_z_nn},
    {"CALL 0x%04X", 3, 24, 2, call_nn},
    {"ADC A,u8", 2, 8, 1, adc_a_n},
    {"RST 08h", 1, 16, 0, rst_08h},
    {"RET NC", 1, 20, 0, ret_nc},
    {"POP DE", 1, 12, 0, pop_de},
    {"JP NC,0x%04X", 3, 16, 2, jp_nc_nn},
    {"UNUSED", 1, 0, 0, NULL},
    {"CALL NC,0x%04X", 3, 24, 2, call_nc_nn},
    {"PUSH DE", 1, 16, 0, push_de},
    {"SUB A,u8", 2, 8, 1, sub_a_n},
    {"RST 10h", 1, 16, 0, rst_10h},
    {"RET C", 1, 20, 0, ret_c},
    {"RETI", 1, 16, 0, reti},
    {"JP C,0x%04X", 3, 16, 2, jp_c_nn},
    {"UNUSED", 1, 0, 0, NULL},
    {"CALL C,0x%04X", 3, 24, 2, call_c_nn},
    {"UNUSED", 1, 0, 0, NULL},
    {"SBC A,u8", 2, 8, 1, sbc_a_n},
    {"RST 18h", 1, 16, 0, rst_18h},
    {"LD (FF00+u8),A", 2, 12, 1, ldh_np_a},
    {"POP HL", 1, 12, 0, pop_hl},
    {"LD (FF00+C),A", 1, 8, 0, ld_cp_a},
    {"UNUSED", 1, 0, 0, NULL},
    {"UNUSED", 1, 0, 0, NULL},
    {"PUSH HL", 1, 16, 0, push_hl},
    {"AND A,u8", 2, 8, 1, and_a_n},
    {"RST 20h", 1, 16, 0, rst_20h},
    {"ADD SP,i8", 2, 16, 1, add_sp_n},
    {"JP HL", 1, 4, 0, jp_hl},
    {"LD (0x%04X),A", 3, 16, 2, ld_nnp_a},
    {"UNUSED", 1, 0, 0, NULL},
    {"UNUSED", 1, 0, 0, NULL},
    {"UNUSED", 1, 0, 0, NULL},
    {"XOR A,u8", 2, 8, 1, xor_a_n},
    {"RST 28h", 1, 16, 0, rst_28h},
    {"LD A,(FF00+u8)", 2, 12, 1, ldh_a_np},
    {"POP AF", 1, 12, 0, pop_af},
    {"LD A,(FF00+C)", 1, 8, 0, ld_a_cp},
    {"DI", 1, 4, 0, di},
    {"UNUSED", 1, 0, 0, NULL},
    {"PUSH AF", 1, 16, 0, push_af},
    {"OR A,u8", 2, 8, 1, or_a_n},
    {"RST 30h", 1, 16, 0, rst_30h},
    {"LD HL,SP+i8", 2, 12, 1, ldhl_sp_n},
    {"LD SP,HL", 1, 8, 0, ld_sp_hl},
    {"LD A,(0x%04X)", 3, 16, 2, ld_a_nnp},
    {"EI", 1, 4, 0, ei},
    {"UNUSED", 1, 0, 0, NULL},
    {"UNUSED", 1, 0, 0, NULL},
    {"CP A,u8", 2, 8, 1, cp_a_n},
    {"RST 38h", 1, 16, 0, rst_38h},
};

const instruction_t bcinstructions[] = {
    {"RLC B", 2, 8, 0, rlc_b},
    {"RLC C", 2, 8, 0, rlc_c},
    {"RLC D", 2, 8, 0, rlc_d},
    {"RLC E", 2, 8, 0, rlc_e},
    {"RLC H", 2, 8, 0, rlc_h},
    {"RLC L", 2, 8, 0, rlc_l},
    {"RLC (HL)", 2, 16, 0, rlc_hlp},
    {"RLC A", 2, 8, 0, rlc_a},
    {"RRC B", 2, 8, 0, rrc_b},
    {"RRC C", 2, 8, 0, rrc_c},
    {"RRC D", 2, 8, 0, rrc_d},
    {"RRC E", 2, 8, 0, rrc_e},
    {"RRC H", 2, 8, 0, rrc_h},
    {"RRC L", 2, 8, 0, rrc_l},
    {"RRC (HL)", 2, 16, 0, rrc_hlp},
    {"RRC A", 2, 8, 0, rrc_a},
    {"RL B", 2, 8, 0, rl_b},
    {"RL C", 2, 8, 0, rl_c},
    {"RL D", 2, 8, 0, rl_d},
    {"RL E", 2, 8, 0, rl_e},
    {"RL H", 2, 8, 0, rl_h},
    {"RL L", 2, 8, 0, rl_l},
    {"RL (HL)", 2, 16, 0, rl_hlp},
    {"RL A", 2, 8, 0, rl_a},
    {"RR B", 2, 8, 0, rr_b},
    {"RR C", 2, 8, 0, rr_c},
    {"RR D", 2, 8, 0, rr_d},
    {"RR E", 2, 8, 0, rr_e},
    {"RR H", 2, 8, 0, rr_h},
    {"RR L", 2, 8, 0, rr_l},
    {"RR (HL)", 2, 16, 0, rr_hlp},
    {"RR A", 2, 8, 0, rr_a},
    {"SLA B", 2, 8, 0, sla_b},
    {"SLA C", 2, 8, 0, sla_c},
    {"SLA D", 2, 8, 0, sla_d},
    {"SLA E", 2, 8, 0, sla_e},
    {"SLA H", 2, 8, 0, sla_h},
    {"SLA L", 2, 8, 0, sla_l},
    {"SLA (HL)", 2, 16, 0, sla_hlp},
    {"SLA A", 2, 8, 0, sla_a},
    {"SRA B", 2, 8, 0, sra_b},
    {"SRA C", 2, 8, 0, sra_c},
    {"SRA D", 2, 8, 0, sra_d},
    {"SRA E", 2, 8, 0, sra_e},
    {"SRA H", 2, 8, 0, sra_h},
    {"SRA L", 2, 8, 0, sra_l},
    {"SRA (HL)", 2, 16, 0, sra_hlp},
    {"SRA A", 2, 8, 0, sra_a},
    {"SWAP B", 2, 8, 0, swap_b},
    {"SWAP C", 2, 8, 0, swap_c},
    {"SWAP D", 2, 8, 0, swap_d},
    {"SWAP E", 2, 8, 0, swap_e},
    {"SWAP H", 2, 8, 0, swap_h},
    {"SWAP L", 2, 8, 0, swap_l},
    {"SWAP (HL)", 2, 16, 0, swap_hlp},
    {"SWAP A", 2, 8, 0, swap_a},
    {"SRL B", 2, 8, 0, srl_b},
    {"SRL C", 2, 8, 0, srl_c},
    {"SRL D", 2, 8, 0, srl_d},
    {"SRL E", 2, 8, 0, srl_e},
    {"SRL H", 2, 8, 0, srl_h},
    {"SRL L", 2, 8, 0, srl_l},
    {"SRL (HL)", 2, 16, 0, srl_hlp},
    {"SRL A", 2, 8, 0, srl_a},
    {"BIT 0,B", 2, 8, 0, bit_0_b},
    {"BIT 0,C", 2, 8, 0, bit_0_c},
    {"BIT 0,D", 2, 8, 0, bit_0_d},
    {"BIT 0,E", 2, 8, 0, bit_0_e},
    {"BIT 0,H", 2, 8, 0, bit_0_h},
    {"BIT 0,L", 2, 8, 0, bit_0_l},
    {"BIT 0,(HL)", 2, 12, 0, bit_0_hlp},
    {"BIT 0,A", 2, 8, 0, bit_0_a},
    {"BIT 1,B", 2, 8, 0, bit_1_b},
    {"BIT 1,C", 2, 8, 0, bit_1_c},
    {"BIT 1,D", 2, 8, 0, bit_1_d},
    {"BIT 1,E", 2, 8, 0, bit_1_e},
    {"BIT 1,H", 2, 8, 0, bit_1_h},
    {"BIT 1,L", 2, 8, 0, bit_1_l},
    {"BIT 1,(HL)", 2, 12, 0, bit_1_hlp},
    {"BIT 1,A", 2, 8, 0, bit_1_a},
    {"BIT 2,B", 2, 8, 0, bit_2_b},
    {"BIT 2,C", 2, 8, 0, bit_2_c},
    {"BIT 2,D", 2, 8, 0, bit_2_d},
    {"BIT 2,E", 2, 8, 0, bit_2_e},
    {"BIT 2,H", 2, 8, 0, bit_2_h},
    {"BIT 2,L", 2, 8, 0, bit_2_l},
    {"BIT 2,(HL)", 2, 12, 0, bit_2_hlp},
    {"BIT 2,A", 2, 8, 0, bit_2_a},
    {"BIT 3,B", 2, 8, 0, bit_3_b},
    {"BIT 3,C", 2, 8, 0, bit_3_c},
    {"BIT 3,D", 2, 8, 0, bit_3_d},
    {"BIT 3,E", 2, 8, 0, bit_3_e},
    {"BIT 3,H", 2, 8, 0, bit_3_h},
    {"BIT 3,L", 2, 8, 0, bit_3_l},
    {"BIT 3,(HL)", 2, 12, 0, bit_3_hlp},
    {"BIT 3,A", 2, 8, 0, bit_3_a},
    {"BIT 4,B", 2, 8, 0, bit_4_b},
    {"BIT 4,C", 2, 8, 0, bit_4_c},
    {"BIT 4,D", 2, 8, 0, bit_4_d},
    {"BIT 4,E", 2, 8, 0, bit_4_e},
    {"BIT 4,H", 2, 8, 0, bit_4_h},
    {"BIT 4,L", 2, 8, 0, bit_4_l},
    {"BIT 4,(HL)", 2, 12, 0, bit_4_hlp},
    {"BIT 4,A", 2, 8, 0, bit_4_a},
    {"BIT 5,B", 2, 8, 0, bit_5_b},
    {"BIT 5,C", 2, 8, 0, bit_5_c},
    {"BIT 5,D", 2, 8, 0, bit_5_d},
    {"BIT 5,E", 2, 8, 0, bit_5_e},
    {"BIT 5,H", 2, 8, 0, bit_5_h},
    {"BIT 5,L", 2, 8, 0, bit_5_l},
    {"BIT 5,(HL)", 2, 12, 0, bit_5_hlp},
    {"BIT 5,A", 2, 8, 0, bit_5_a},
    {"BIT 6,B", 2, 8, 0, bit_6_b},
    {"BIT 6,C", 2, 8, 0, bit_6_c},
    {"BIT 6,D", 2, 8, 0, bit_6_d},
    {"BIT 6,E", 2, 8, 0, bit_6_e},
    {"BIT 6,H", 2, 8, 0, bit_6_h},
    {"BIT 6,L", 2, 8, 0, bit_6_l},
    {"BIT 6,(HL)", 2, 12, 0, bit_6_hlp},
    {"BIT 6,A", 2, 8, 0, bit_6_a},
    {"BIT 7,B", 2, 8, 0, bit_7_b},
    {"BIT 7,C", 2, 8, 0, bit_7_c},
    {"BIT 7,D", 2, 8, 0, bit_7_d},
    {"BIT 7,E", 2, 8, 0, bit_7_e},
    {"BIT 7,H", 2, 8, 0, bit_7_h},
    {"BIT 7,L", 2, 8, 0, bit_7_l},
    {"BIT 7,(HL)", 2, 12, 0, bit_7_hlp},
    {"BIT 7,A", 2, 8, 0, bit_7_a},
    {"RES 0,B", 2, 8, 0, res_0_b},
    {"RES 0,C", 2, 8, 0, res_0_c},
    {"RES 0,D", 2, 8, 0, res_0_d},
    {"RES 0,E", 2, 8, 0, res_0_e},
    {"RES 0,H", 2, 8, 0, res_0_h},
    {"RES 0,L", 2, 8, 0, res_0_l},
    {"RES 0,(HL)", 2, 16, 0, res_0_hlp},
    {"RES 0,A", 2, 8, 0, res_0_a},
    {"RES 1,B", 2, 8, 0, res_1_b},
    {"RES 1,C", 2, 8, 0, res_1_c},
    {"RES 1,D", 2, 8, 0, res_1_d},
    {"RES 1,E", 2, 8, 0, res_1_e},
    {"RES 1,H", 2, 8, 0, res_1_h},
    {"RES 1,L", 2, 8, 0, res_1_l},
    {"RES 1,(HL)", 2, 16, 0, res_1_hlp},
    {"RES 1,A", 2, 8, 0, res_1_a},
    {"RES 2,B", 2, 8, 0, res_2_b},
    {"RES 2,C", 2, 8, 0, res_2_c},
    {"RES 2,D", 2, 8, 0, res_2_d},
    {"RES 2,E", 2, 8, 0, res_2_e},
    {"RES 2,H", 2, 8, 0, res_2_h},
    {"RES 2,L", 2, 8, 0, res_2_l},
    {"RES 2,(HL)", 2, 16, 0, res_2_hlp},
    {"RES 2,A", 2, 8, 0, res_2_a},
    {"RES 3,B", 2, 8, 0, res_3_b},
    {"RES 3,C", 2, 8, 0, res_3_c},
    {"RES 3,D", 2, 8, 0, res_3_d},
    {"RES 3,E", 2, 8, 0, res_3_e},
    {"RES 3,H", 2, 8, 0, res_3_h},
    {"RES 3,L", 2, 8, 0, res_3_l},
    {"RES 3,(HL)", 2, 16, 0, res_3_hlp},
    {"RES 3,A", 2, 8, 0, res_3_a},
    {"RES 4,B", 2, 8, 0, res_4_b},
    {"RES 4,C", 2, 8, 0, res_4_c},
    {"RES 4,D", 2, 8, 0, res_4_d},
    {"RES 4,E", 2, 8, 0, res_4_e},
    {"RES 4,H", 2, 8, 0, res_4_h},
    {"RES 4,L", 2, 8, 0, res_4_l},
    {"RES 4,(HL)", 2, 16, 0, res_4_hlp},
    {"RES 4,A", 2, 8, 0, res_4_a},
    {"RES 5,B", 2, 8, 0, res_5_b},
    {"RES 5,C", 2, 8, 0, res_5_c},
    {"RES 5,D", 2, 8, 0, res_5_d},
    {"RES 5,E", 2, 8, 0, res_5_e},
    {"RES 5,H", 2, 8, 0, res_5_h},
    {"RES 5,L", 2, 8, 0, res_5_l},
    {"RES 5,(HL)", 2, 16, 0, res_5_hlp},
    {"RES 5,A", 2, 8, 0, res_5_a},
    {"RES 6,B", 2, 8, 0, res_6_b},
    {"RES 6,C", 2, 8, 0, res_6_c},
    {"RES 6,D", 2, 8, 0, res_6_d},
    {"RES 6,E", 2, 8, 0, res_6_e},
    {"RES 6,H", 2, 8, 0, res_6_h},
    {"RES 6,L", 2, 8, 0, res_6_l},
    {"RES 6,(HL)", 2, 16, 0, res_6_hlp},
    {"RES 6,A", 2, 8, 0, res_6_a},
    {"RES 7,B", 2, 8, 0, res_7_b},
    {"RES 7,C", 2, 8, 0, res_7_c},
    {"RES 7,D", 2, 8, 0, res_7_d},
    {"RES 7,E", 2, 8, 0, res_7_e},
    {"RES 7,H", 2, 8, 0, res_7_h},
    {"RES 7,L", 2, 8, 0, res_7_l},
    {"RES 7,(HL)", 2, 16, 0, res_7_hlp},
    {"RES 7,A", 2, 8, 0, res_7_a},
    {"SET 0,B", 2, 8, 0, set_0_b},
    {"SET 0,C", 2, 8, 0, set_0_c},
    {"SET 0,D", 2, 8, 0, set_0_d},
    {"SET 0,E", 2, 8, 0, set_0_e},
    {"SET 0,H", 2, 8, 0, set_0_h},
    {"SET 0,L", 2, 8, 0, set_0_l},
    {"SET 0,(HL)", 2, 16, 0, set_0_hlp},
    {"SET 0,A", 2, 8, 0, set_0_a},
    {"SET 1,B", 2, 8, 0, set_1_b},
    {"SET 1,C", 2, 8, 0, set_1_c},
    {"SET 1,D", 2, 8, 0, set_1_d},
    {"SET 1,E", 2, 8, 0, set_1_e},
    {"SET 1,H", 2, 8, 0, set_1_h},
    {"SET 1,L", 2, 8, 0, set_1_l},
    {"SET 1,(HL)", 2, 16, 0, set_1_hlp},
    {"SET 1,A", 2, 8, 0, set_1_a},
    {"SET 2,B", 2, 8, 0, set_2_b},
    {"SET 2,C", 2, 8, 0, set_2_c},
    {"SET 2,D", 2, 8, 0, set_2_d},
    {"SET 2,E", 2, 8, 0, set_2_e},
    {"SET 2,H", 2, 8, 0, set_2_h},
    {"SET 2,L", 2, 8, 0, set_2_l},
    {"SET 2,(HL)", 2, 16, 0, set_2_hlp},
    {"SET 2,A", 2, 8, 0, set_2_a},
    {"SET 3,B", 2, 8, 0, set_3_b},
    {"SET 3,C", 2, 8, 0, set_3_c},
    {"SET 3,D", 2, 8, 0, set_3_d},
    {"SET 3,E", 2, 8, 0, set_3_e},
    {"SET 3,H", 2, 8, 0, set_3_h},
    {"SET 3,L", 2, 8, 0, set_3_l},
    {"SET 3,(HL)", 2, 16, 0, set_3_hlp},
    {"SET 3,A", 2, 8, 0, set_3_a},
    {"SET 4,B", 2, 8, 0, set_4_b},
    {"SET 4,C", 2, 8, 0, set_4_c},
    {"SET 4,D", 2, 8, 0, set_4_d},
    {"SET 4,E", 2, 8, 0, set_4_e},
    {"SET 4,H", 2, 8, 0, set_4_h},
    {"SET 4,L", 2, 8, 0, set_4_l},
    {"SET 4,(HL)", 2, 16, 0, set_4_hlp},
    {"SET 4,A", 2, 8, 0, set_4_a},
    {"SET 5,B", 2, 8, 0, set_5_b},
    {"SET 5,C", 2, 8, 0, set_5_c},
    {"SET 5,D", 2, 8, 0, set_5_d},
    {"SET 5,E", 2, 8, 0, set_5_e},
    {"SET 5,H", 2, 8, 0, set_5_h},
    {"SET 5,L", 2, 8, 0, set_5_l},
    {"SET 5,(HL)", 2, 16, 0, set_5_hlp},
    {"SET 5,A", 2, 8, 0, set_5_a},
    {"SET 6,B", 2, 8, 0, set_6_b},
    {"SET 6,C", 2, 8, 0, set_6_c},
    {"SET 6,D", 2, 8, 0, set_6_d},
    {"SET 6,E", 2, 8, 0, set_6_e},
    {"SET 6,H", 2, 8, 0, set_6_h},
    {"SET 6,L", 2, 8, 0, set_6_l},
    {"SET 6,(HL)", 2, 16, 0, set_6_hlp},
    {"SET 6,A", 2, 8, 0, set_6_a},
    {"SET 7,B", 2, 8, 0, set_7_b},
    {"SET 7,C", 2, 8, 0, set_7_c},
    {"SET 7,D", 2, 8, 0, set_7_d},
    {"SET 7,E", 2, 8, 0, set_7_e},
    {"SET 7,H", 2, 8, 0, set_7_h},
    {"SET 7,L", 2, 8, 0, set_7_l},
    {"SET 7,(HL)", 2, 16, 0, set_7_hlp},
    {"SET 7,A", 2, 8, 0, set_7_a},
};