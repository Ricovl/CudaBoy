#pragma once

#include <fstream>
#include <cstdio>
#include "config.hpp"

#define ROM_ENTRY_OFFSET    0x100
#define ROM_TITLE_OFFSET    0x134
#define ROM_TYPE_OFFSET     0x147
#define ROM_ROM_SIZE_OFFSET 0x148
#define ROM_RAM_SIZE_OFFSET 0x149

int load_rom(uint8_t **buf) {
    std::ifstream file("/home/rico/Documents/gameboy/games/tetris.gb");
    // std::ifstream file("/home/rico/Documents/gameboy/games/10-bitops.gb");

    if (!file) {
        printf("Error opening rom file!\n");
        return -1;
    }

    file.seekg (0, file.end);
    int length = file.tellg();
    file.seekg (0, file.beg);

    printf("rom size: %d\n", length);

    char *buffer = (char*)malloc(length);
    file.read(buffer, length);

    char *rom_title = &buffer[ROM_TITLE_OFFSET];

    unsigned cartridge_rom_size = (1<<15) << buffer[ROM_ROM_SIZE_OFFSET];
    unsigned cartridge_ram_size = buffer[ROM_RAM_SIZE_OFFSET];

    printf("cartridge title: %s\n", rom_title);
    printf("cartridge rom size: %04x\n", cartridge_rom_size);
    printf("cartridge ram size: %04x\n", cartridge_ram_size);

    *buf = (uint8_t*)buffer;
    file.close();
    return 0;
}