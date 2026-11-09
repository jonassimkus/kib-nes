#include "core/headers/cpu.h"
#include "core/headers/bus.h"
#include <cstdint>
#include <cstdio>

cpu::cpu(){
    m_bus = new bus();
}

void cpu::load_cartridge(cartridge *cart){
    m_bus->load_cartridge(cart);
    power_on();
}

void cpu::write_mem(uint16_t addr, uint8_t val){
    m_bus->write_mem(addr, val);
}

uint8_t cpu::read_mem(uint16_t addr){
    return m_bus->read_mem(addr);
}

void cpu::opcode_message(){
    std::cout << "Opcode ";
    printf("%04X", read_mem(program_counter-1));
    std::cout << " not defined" << std::endl;
}

void cpu::cycle(){
    if (cpu_state != CPU_RUNNING){
        return;
    }
    
    if (cpu_state == CPU_PAUSED){
        return;
    }

    uint8_t opcode = read_mem(program_counter);
    program_counter += 1;

    switch (opcode) {
        case 0x4C:{
            uint8_t low_byte = read_mem(program_counter);
            uint8_t high_byte = read_mem(program_counter+1);
            uint16_t address = (high_byte << 8) | low_byte;

            program_counter = address;
            break;
        }

        case 0xA2: {
            x_register = read_mem(program_counter+1);
            program_counter+=1;
            break;
        }

        default:{
            opcode_message();
            cpu_state = CPU_CRASHED;
            break;
        }
    }
}


// First instruction at 0xFFFC and 0xFFFD in pgr

void cpu::power_on(){
    uint8_t low_byte = read_mem(0xFFFC);
    uint8_t high_byte = read_mem(0xFFFD);

    uint16_t address = (high_byte << 8) | low_byte;
    program_counter = address;
    
    stack_pointer = 0x00FD;
    proccesor_status_register = 0x0024;
    accumulator = 0;
    x_register = 0;
    y_register = 0;

    // set program counter to 0xc000 for nestest automating
    program_counter = 0xC000;

    cpu_state = CPU_RUNNING;
}
