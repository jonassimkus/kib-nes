#include "core/headers/cpu.h"

void cpu::write_mem(uint16_t addr, uint8_t val){
    m_bus->write_mem(addr, val);
}

uint8_t cpu::read_mem(uint16_t addr){
    return m_bus->read_mem(addr);
}