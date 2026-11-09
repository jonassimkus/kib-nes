#pragma  once

#include <cstdint>
#include "./cartridge.h"

class ppu_bus{
    private:
    
    uint8_t nametable[2048] = {};
    uint8_t palette[32] = {};
    cartridge *cart = nullptr;

    public:

    void load_cartridge(cartridge *c){
        cart = c;
    }

    void write_mem(uint16_t addr, uint8_t val);
    uint8_t read_mem(uint16_t addr);

    uint8_t palette_index(uint16_t addr);
    uint16_t nametable_index(uint16_t addr);

};