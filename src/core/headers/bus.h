#pragma  once

#include <cstdint>
#include "./cartridge.h"
#include "./ppu.h"

class bus{
    private:
    
    // Stack $0100–$01FF
    uint8_t memory[2048] = {};
    cartridge *cart = nullptr;
    ppu *m_ppu = nullptr;

    public:

    void load_ppu(ppu*new_ppu){
        m_ppu = new_ppu;
    }

    void load_cartridge(cartridge *c){
        cart = c;
    }

    void write_mem(uint16_t addr, uint8_t val);
    uint8_t read_mem(uint16_t addr);
};