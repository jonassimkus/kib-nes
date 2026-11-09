#include "core/headers/bus.h"
#include <cstdint>

void bus::write_mem(uint16_t addr, uint8_t val){
    if (addr >= 0x4020 && cart != nullptr){
        cart->write_mem(addr, val);
    }

    if (addr >= 0x2000 && addr <= 0x3FFF && m_ppu != nullptr){
        m_ppu->cpu_write(addr & 0x7, val);
    }

    if (addr <= 0x1FFF){
        memory[addr & 0x07FF] = val;
    }
}

uint8_t bus::read_mem(uint16_t addr){
    if (addr >= 0x4020 && cart != nullptr){
        return cart->read_mem(addr);
    }

    if (addr >= 0x2000 && addr <= 0x3FFF && m_ppu != nullptr){
        return m_ppu->cpu_read(addr & 0x7);
    }

    if (addr <= 0x1FFF){
        return memory[addr & 0x07FF];
    }

    return 0;
}
