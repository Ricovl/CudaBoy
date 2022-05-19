#pragma once

#include <iostream>

#include "instructions.hpp"
#include "state.hpp"
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

static void step(state_t &s) {
    if (s.inst_cycles_wait <= 21) {
        _inst_t opcode = read_u8(s, s.pc);
        auto inst = instructions[opcode];

        if (inst.execute != nullptr) {
            
            if (inst.length == 2) {
                s.operand = read_u8(s, s.pc + 1);
            }
            else if (inst.length == 3) {
                s.operand = read_u8(s, s.pc + 1) | read_u8(s, s.pc + 2) << 8;
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


            s.num_inst++;
        }
        else {
            std::cout << "Error: instruction not implemented" << opcode << "\n";
            s.halt = true;
        }
    }
    else {
        s.inst_cycles_wait -= 1;
    }
    s.cycles += 1;
}

