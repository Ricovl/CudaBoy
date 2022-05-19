#pragma once

#include <cstdint>
#include <cstring>

#include "rom.hpp"
#include "config.hpp"
#include "lcd.hpp"

typedef uint8_t _inst_t;
typedef uint8_t _op8_t;
typedef uint16_t _op16_t;
typedef uint8_t _reg8_t;
typedef uint16_t _reg16_t;


typedef struct registers_t {
    union {
        struct __attribute__((__packed__)) {
            union {
                struct __attribute__((__packed__))
                {
                    _reg8_t und : 4;
                    _reg8_t carry : 1;
                    _reg8_t half_carry : 1;
                    _reg8_t subtract : 1;
                    _reg8_t zero : 1;
                };
                _reg8_t f;
            };
            _reg8_t a;
        };
        _reg16_t af;
    };

    union {
        struct {
            _reg8_t c;
            _reg8_t b;
        };
        _reg16_t bc;
    };

    union {
        struct {
            _reg8_t e;
            _reg8_t d;
        };
        _reg16_t de;
    };

    union {
        struct {
            _reg8_t l;
            _reg8_t h;
        };
        _reg16_t hl;
    };

    _reg16_t sp;
};

typedef struct flagsreg_t {
    _reg8_t und : 4;
    _reg8_t zero : 1;
    _reg8_t subtract : 1;
    _reg8_t half_carry : 1;
    _reg8_t carry : 1;
};

typedef struct state_t {
    registers_t regs;
    _reg16_t pc;
    _op16_t operand;

    bool halt;
    bool stop;

    bool interrupts_enabled;
    lcd_t lcd;

    unsigned cycles;
    unsigned inst_cycles_wait;
    bool prefixed;

    unsigned rom_size;
    uint8_t *mem;

    bool breakp;
    unsigned num_inst;
};

static void initialize_state(state_t* &s, uint8_t *rom) {
    s = (state_t*)malloc(sizeof(state_t));

    memset(&s->regs, 0, sizeof(registers_t));

    s->pc = ROM_ENTRY_OFFSET;
#if USE_BOOTROM
    s->pc = 0;
#endif
    s->halt = false;
    s->stop = false;
    s->cycles = 0;
    s->inst_cycles_wait = 0;
    s->prefixed = false;
    s->rom_size = (1<<15) << rom[ROM_ROM_SIZE_OFFSET];
    s->mem = (uint8_t *)malloc(0x10000);
    memcpy(s->mem, rom, s->rom_size);

    memset(&s->regs, 0, sizeof(register_t));
    s->regs.af = 0x01b0;
    s->regs.bc = 0x0013;
    s->regs.de = 0x00d8;
    s->regs.hl = 0x014d;
    s->regs.sp = 0xfffe;

    s->breakp = false;
    s->num_inst = 0;
}