#include "core/headers/bus.h"

void bus::write_mem(uint16_t addr, uint8_t val){
    memory[addr] = val;
}

uint8_t bus::read_mem(uint16_t addr){
    return memory[addr];
}