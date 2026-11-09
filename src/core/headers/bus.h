#pragma  once

#include <cstdint>
#include "./cartridge.h"

class bus{
    private:
    
    uint8_t memory[2048] = {};
    cartridge *cart = nullptr;

    public:

    void write_mem(uint16_t addr, uint8_t val);
    uint8_t read_mem(uint16_t addr);

    void write_chr(uint16_t addr, uint8_t val);
    uint8_t read_chr(uint16_t addr);
};