#include "core/headers/bus.h"
#include <cstdint>

void bus::write_mem(uint16_t addr, uint8_t val){
    if (addr >= 0x4020 && cart != nullptr){
        cart->write_mem(addr, val);
    }

    if (addr <= 0x1FFF){
        memory[addr & 0x07FF] = val;
    }
}

uint8_t bus::read_mem(uint16_t addr){
    if (addr >= 0x4020 && cart != nullptr){
        return cart->read_mem(addr);
    }

    if (addr <= 0x1FFF){
        return memory[addr & 0x07FF];
    }

    return 0;
}

void bus::write_chr(uint16_t addr, uint8_t val){
    cart->write_chr(addr, val);
}

uint8_t bus::read_chr(uint16_t addr){
    return cart->read_chr(addr);
}