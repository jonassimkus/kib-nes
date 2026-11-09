#include "core/headers/ppu_bus.h"
#include <cstdint>

uint16_t ppu_bus::nametable_index(uint16_t addr){
    addr &= 0x0FFF;

    if (cart != nullptr && cart->is_vertical_mirroring()){
        return addr & 0x07FF;
    }

    return ((addr >> 1) & 0x0400) | (addr & 0x03FF);
}

uint8_t ppu_bus::palette_index(uint16_t addr){
    uint8_t index = addr & 0x1F;

    if (index == 0x10 || index == 0x14 || index == 0x18 || index == 0x1C){
        index -= 0x10;
    }

    return index;
}

void ppu_bus::write_mem(uint16_t addr, uint8_t val){
    addr &= 0x3FFF;

    if (addr <= 0x1FFF){
        if (cart != nullptr){
            cart->write_chr(addr, val);
        }
    }else if (addr <= 0x3EFF){
        nametable[nametable_index(addr)] = val;
    }else{
        palette[palette_index(addr)] = val;
    }
}

uint8_t ppu_bus::read_mem(uint16_t addr){
    addr &= 0x3FFF;

    if (addr <= 0x1FFF){
        if (cart != nullptr){
            return cart->read_chr(addr);
        }
        
        return 0;
    }else if (addr <= 0x3EFF){
        return nametable[nametable_index(addr)];
    }

    return palette[palette_index(addr)];
}