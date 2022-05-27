#pragma once

#include <cstdint>
#include <cstring>

#include "config.hpp"
#include "lcd_state.hpp"

typedef uint8_t _inst_t;
typedef uint8_t _op8_t;
typedef uint16_t _op16_t;
typedef uint8_t _reg8_t;
typedef uint16_t _reg16_t;

enum
{
    P1   = 0xff00,    // Joypad (R/W)
    SB   = 0xff01,    // Serial transer data (R/W)
    SC   = 0xff02,    // SIO Control (R/W)

    DIV  = 0xff04,   // Divider Register (R/W)
    TIMA = 0xff05,   // Timer counter (R/W)
    TMA  = 0xff06,   // Timer Modulo (R/W)
    TAC  = 0xff07,   // Timer Control (R/W)
    
    IF   = 0xff0f,   // Interrupt Flag (R/W)

    LCDC = 0xff40,   // LCD Control (R/W)
    STAT = 0xff41,   // LCD Status (R/W)
    SCY  = 0xff42,   // Scroll Y (R/W)
    SCX  = 0xff43,   // Scroll X (R/W)
    LY   = 0xff44,   // LCDC Y-Coordinate (R)
    LYC  = 0xff45,   // LY Compare (R/W)

    DMA  = 0xff46,   // DMA Transfer and Start Address (W)

    BGP  = 0xff47,   // BG & Window Palette Data (R/W)
    OBP0 = 0xff48,   // Object Palette 0 Data (R/W)
    OBP1 = 0xff49,   // Object Palette 1 Data (R/W)
    WY   = 0xff4A,   // Window Y Position (R/W)
    WX   = 0xff4B,   // Window X Position (R/W)

    IE   = 0xffff,   // Interrupt Enable (R/W)
};

struct registers_t {
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

struct flagsreg_t {
    _reg8_t und : 4;
    _reg8_t zero : 1;
    _reg8_t subtract : 1;
    _reg8_t half_carry : 1;
    _reg8_t carry : 1;
};

struct state_t {
    registers_t regs;
    _reg16_t pc;
    _op16_t operand;

    bool halt;
    bool stop;

    bool interrupts_enabled;
    lcd_t lcd;

    unsigned cycles;
    int inst_cycles_wait;
    bool prefixed;

    _reg16_t divider_counter;
    _reg16_t timer_counter;
    _reg8_t timer_control;

    unsigned rom_size;
    uint8_t *mem;

    bool breakp;
    unsigned num_inst;
};

