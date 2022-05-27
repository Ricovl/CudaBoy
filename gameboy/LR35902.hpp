#pragma once

#include <iostream>

#include "instructions.hpp"
#include "state.hpp"
#include "lcd_ctrl.hpp"
#include "config.hpp"

static void execute() {
    
}

static void do_debug_stuff(state_t &s) {
            // printf("pc: %04x, %02x(%02x%02x)\n", s.pc, read_u8(s, s.pc), read_u8(s, s.pc + 1), read_u8(s, s.pc + 2));
            // printf("A: %02X F: %02X B: %02X C: %02X D: %02X E: %02X H: %02X L: %02X SP: %02X PC: 00:%04X (%02X %02X %02X %02X)\n", 
            // s.regs.a, s.regs.f, s.regs.b, s.regs.c, s.regs.d, s.regs.e, s.regs.h, s.regs.l, s.regs.sp, s.pc, read_u8(s, s.pc), read_u8(s, s.pc + 1), read_u8(s, s.pc + 2), read_u8(s, s.pc + 3));

#if DEBUG
            if (s.pc == 0xcaf6) // runs okay till at least caf6 some screen stuff happens here.
                s.breakp = true;
            print_debug(s);
            printf("00:%04x:  %02x", s.pc, opcode);
            if (inst.length > 1) {
                printf("%02x", read_u8(s, s.pc + 1));
            }
            if (inst.length > 2) {
                printf("%02x", read_u8(s, s.pc + 2));
            }
            std::cout << "     " << inst.name << "\n";
            if (s.breakp) getc(stdin);
#endif
}

static void print_debug(state_t &s) {
    printf("A: %02X  F: %02X  (AF: %04X)\n", s.regs.a, s.regs.f, s.regs.af);
    printf("B: %02X  C: %02X  (BC: %04X)\n", s.regs.b, s.regs.c, s.regs.bc);
    printf("D: %02X  E: %02X  (DE: %04X)\n", s.regs.d, s.regs.e, s.regs.de);
    printf("H: %02X  L: %02X  (HL: %04X)\n", s.regs.h, s.regs.l, s.regs.hl);
    printf("PC: %04X  SP: %04X\n", s.pc, s.regs.sp);
    char zero = (s.regs.zero) ? 'Z' : '-';
    char nega = (s.regs.subtract) ? 'N' : '-';
    char half = (s.regs.half_carry) ? 'H' : '-';
    char carr = (s.regs.carry) ? 'C' : '-';
    printf("F: [%c%c%c%c]\n", zero, nega, half, carr);
    printf("T-cycle: %d\n", s.cycles);
}

void timer_cycle(state_t &s) {

    if (s.timer_counter <= 0) {
        uint8_t tac = read_u8(s, TAC);
        s.timer_counter = 0;
    }
    s.timer_counter--;
}

static void step(state_t &s) {
    s.cycles += 1;

    lcd_cycle(s);

    timer_cycle(s);

    s.inst_cycles_wait -= 1;
    if (s.inst_cycles_wait <= 0)
    {
        if (s.halt) {
            s.inst_cycles_wait = 4;
            return;
        }

        _inst_t opcode = read_u8(s, s.pc);
        auto inst = instructions[opcode];

        if (inst.execute != nullptr) {
            
            if (inst.length == 2) {
                s.operand = read_u8(s, s.pc + 1);
            }
            else if (inst.length == 3) {
                s.operand = read_u16(s, s.pc + 1);
            }

            do_debug_stuff(s);

            s.pc += inst.length;
            s.inst_cycles_wait = inst.cycles;

            inst.execute(s);

            if (s.prefixed) {
                opcode = read_u8(s, s.pc);
                inst = bcinstructions[opcode];
                inst.execute(s);
                s.inst_cycles_wait += inst.cycles;
                s.pc++;
                s.prefixed = false;
            }
        }
        else {
            std::cout << "Error: instruction not implemented" << opcode << "\n";
            s.halt = true;
        }
    }
}

static void initialize_state(state_t* &s, uint8_t *rom) {
    s = (state_t*)malloc(sizeof(state_t));

    memset(&s->regs, 0, sizeof(registers_t));

    s->pc = 0x100;
#if USE_BOOTROM
    s->pc = 0;
#endif
    s->halt = false;
    s->stop = false;
    s->cycles = 0;
    s->inst_cycles_wait = 0;
    s->prefixed = false;
    s->rom_size = (1<<15) << rom[0x0148];
    s->mem = (uint8_t *)malloc(0x10000);
    memcpy(s->mem, rom, s->rom_size);

    memset(&s->regs, 0, sizeof(register_t));
    lcd_init(*s);

    s->regs.af = 0x01b0;
    s->regs.bc = 0x0013;
    s->regs.de = 0x00d8;
    s->regs.hl = 0x014d;
    s->regs.sp = 0xfffe;

    write_u8(*s, TIMA, 0x00);
    write_u8(*s, TMA,  0x00);
    write_u8(*s, TAC,  0x00);
    write_u8(*s, LCDC, 0x91);
    write_u8(*s, SCY,  0x00);
    write_u8(*s, SCX,  0x00);
    write_u8(*s, LYC,  0x00);
    write_u8(*s, BGP,  0xfc);
    write_u8(*s, OBP0, 0xff);
    write_u8(*s, OBP1, 0xff);
    write_u8(*s, WY,   0x00);
    write_u8(*s, WX,   0x00);
    write_u8(*s, IE,   0x00);

    s->breakp = false;
    s->num_inst = 0;
}