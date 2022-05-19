#pragma once

#include <fstream>

#define ROM_ENTRY_OFFSET    0x100
#define ROM_TITLE_OFFSET    0x134
#define ROM_TYPE_OFFSET     0x147
#define ROM_ROM_SIZE_OFFSET 0x148
#define ROM_RAM_SIZE_OFFSET 0x149

int load_rom(uint8_t **buf) {
    std::ifstream file("/home/rico/Documents/gameboy/games/01-special.gb");

    if (!file) {
        std::cout << "Error opening rom file" << "\n";
        return -1;
    }

    file.seekg (0, file.end);
    int length = file.tellg();
    file.seekg (0, file.beg);

    std::cout << "rom size: " << length << "\n";

    char *buffer = (char*)malloc(length);
    file.read(buffer, length);

    char *rom_title = &buffer[ROM_TITLE_OFFSET];

    unsigned cartridge_rom_size = (1<<15) << buffer[ROM_ROM_SIZE_OFFSET];
    unsigned cartridge_ram_size = buffer[ROM_RAM_SIZE_OFFSET];

    std::cout << "cartridge title: " << rom_title << "\n";
    std::cout << "cartridge rom size: " << cartridge_rom_size << "\n";
    std::cout << "cartridge ram size: " << cartridge_ram_size << "\n";

    *buf = (uint8_t*)buffer;
    file.close();
    return 0;
}