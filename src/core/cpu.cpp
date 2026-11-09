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

void cpu::load_ppu(ppu *new_ppu){
    m_bus->load_ppu(new_ppu);
}

int cpu::get_cpu_state(){
    return cpu_state;
}

void cpu::push(uint8_t val){
    write_mem(0x100 + stack_pointer, val);
    stack_pointer -= 1;
}

uint8_t cpu::pull(){
    stack_pointer += 1;
    return read_mem(0x100 + stack_pointer);
}

void cpu::write_mem(uint16_t addr, uint8_t val){
    m_bus->write_mem(addr, val);
}

uint8_t cpu::read_mem(uint16_t addr){
    return m_bus->read_mem(addr);
}

void cpu::set_flag(uint8_t mask, bool val){
    if (val){
        proccesor_status_register |= mask;
    }else{
        proccesor_status_register &= ~mask;
    }
}

bool cpu::get_flag(uint8_t mask){
    return proccesor_status_register & mask;
}

void cpu::set_zn(uint8_t val){
    if (val == 0){
        set_flag(FLAG_Z, true);
    }else{
        set_flag(FLAG_Z, false);
    }
    if ((val & 0x80) != 0){
        set_flag(FLAG_N, true);
    }else{
        set_flag(FLAG_N, false);
    }
}

void cpu::opcode_message(){
    std::cout << "Opcode $";
    printf("%02X", read_mem(program_counter-1));
    std::cout << " not defined" << std::endl;
}

void cpu::log(){
    std::cout << "$";
    printf("%04X", (program_counter));
    std::cout << " A:";    
    printf("%02X", (accumulator));
    std::cout << " X:";    
    printf("%02X", (x_register));
    std::cout << " Y:";    
    printf("%02X", (y_register));
    std::cout << " P:";    
    printf("%02X", (proccesor_status_register));
    std::cout << " SP:";    
    printf("%02X", (stack_pointer));
    std::cout << " CYC:" << cycles << std::endl;  
}

void cpu::branch(bool f){
    cycles+=2;
    int8_t byte = (int8_t)read_mem(program_counter);
    program_counter += 1;
    uint8_t original_high_byte = (program_counter) >> 8;

    if (f){
        cycles+=1;
        program_counter += byte;
        if (original_high_byte != (program_counter >> 8)){
            cycles+=1;
        }
    }
}

void cpu::cycle(){
    if (cpu_state != CPU_RUNNING){
        return;
    }
    
    if (cpu_state == CPU_PAUSED){
        return;
    }

    log();

    uint8_t opcode = read_mem(program_counter);
    program_counter += 1;

    switch (opcode) {
        //NOP
        case 0xEA:{
            cycles+=2;
            break;
        }

        //SEC
        case 0x38:{
            cycles += 2;
            set_flag(FLAG_C, true);
            break;
        }

        //CLC
        case 0x18:{
            cycles += 2;
            set_flag(FLAG_C, false);
            break;
        }

        //BCS
        case 0xB0:{
            branch(get_flag(FLAG_C));
            break;
        }

        //BCC
        case 0x90:{
            branch(!get_flag(FLAG_C));
            break;
        }

        //BEQ
        case 0xF0:{
            branch(get_flag(FLAG_Z));
            break;
        }

        //BNE
        case 0xD0:{
            branch(!get_flag(FLAG_Z));
            break;
        }

        //BVS
        case 0x70:{
            branch(get_flag(FLAG_V));
            break;
        }

        //BVC
        case 0x50:{
            branch(!get_flag(FLAG_V));
            break;
        }

        //BPL
        case 0x10:{
            branch(!get_flag(FLAG_N));
            break;
        }

        //BMI
        case 0x30:{
            branch(get_flag(FLAG_N));
            break;
        }

        //BIT
        case 0x24:{
            cycles +=3;
            uint8_t value = read_mem(read_mem(program_counter));
            uint8_t new_memory = accumulator & value;

            set_flag(FLAG_Z, new_memory == 0);
            set_flag(FLAG_N, (value & 0x80) != 0);
            set_flag(FLAG_V, (value & 0x40) != 0);
            program_counter +=1;

            break;
        }

        //BIT absolute
        case 0x2C:{
            cycles +=4;
            uint8_t value = read_mem((read_mem(program_counter+1) << 8) | read_mem(program_counter));
            uint8_t new_memory = accumulator & value;

            set_flag(FLAG_Z, new_memory == 0);
            set_flag(FLAG_N, (value & 0x80) != 0);
            set_flag(FLAG_V, (value & 0x40) != 0);
            program_counter +=2;

            break;
        }

        //RTS
        case 0x60:{
            uint8_t low_byte = pull();
            uint8_t high_byte = pull();

            uint16_t address = (high_byte << 8) | low_byte;
            program_counter = address + 1;
            cycles+=6;
            break;
        }

        //RTI
        case 0x40:{
            uint8_t values = pull();
            uint8_t low_byte = pull();
            uint8_t high_byte = pull();
            uint16_t address = (high_byte << 8) | low_byte;

            set_flag(FLAG_C, values & 0x1);
            set_flag(FLAG_Z, values & 0x2);
            set_flag(FLAG_I, values & 0x4);
            set_flag(FLAG_D, values & 0x8);
            set_flag(FLAG_V, values & 0x40);
            set_flag(FLAG_N, values & 0x80);

            program_counter  = address;
            cycles+=6;
            break;
        }

        //LSR - Accumulator
        case 0x4A:{
            set_flag(FLAG_C, accumulator & 0x1);
            accumulator >>= 1;
            set_zn(accumulator);
            cycles+=2;
            break;
        }

        //LSR - ZERO
        case 0x46:{
            uint8_t address = read_mem(program_counter);
            uint8_t value = read_mem(address);

            set_flag(FLAG_C, value & 0x1);
            value >>= 1;
            set_zn(value);
            write_mem(address,value);

            program_counter+=1;
            cycles+=5;
            break;
        }

        //ASL - Accumulator
        case 0x0A:{
            set_flag(FLAG_C, accumulator & 0x80);
            accumulator <<= 1;
            set_zn(accumulator);
            cycles+=2;
            break;
        }

        //ASL - ZERO
        case 0x06:{
            uint8_t address = read_mem(program_counter);
            uint8_t value = read_mem(address);

            set_flag(FLAG_C, value & 0x80);
            value <<= 1;
            set_zn(value);
            write_mem(address, value);

            cycles+=5;
            program_counter+=1;
            break;
        }


        //ROR - Accumulator
        case 0x6A:{
            uint8_t old_c = get_flag(FLAG_C);
            set_flag(FLAG_C, accumulator & 0x1);
            accumulator = accumulator >> 1;
            if (old_c == 1){
                accumulator |= 0x80;
            }
            set_zn(accumulator);
            cycles+=2;
            break;
        }


        //ROR - Zero
        case 0x66:{
            uint8_t address = read_mem(program_counter);
            uint8_t value = read_mem(address);

            uint8_t old_c = get_flag(FLAG_C);
            set_flag(FLAG_C, value & 0x1);
            value = value >> 1;
            if (old_c == 1){
                value |= 0x80;
            }
            
            set_zn(value);
            write_mem(address, value);

            cycles+=5;
            program_counter+=1;
            break;
        }


        //ROL - Accumulator
        case 0x2A:{
            uint8_t old_c = get_flag(FLAG_C);
            set_flag(FLAG_C, accumulator & 0x80);
            accumulator = accumulator << 1;
            if (old_c == 1){
                accumulator |= 0x1;
            }
            set_zn(accumulator);
            cycles+=2;
            break;
        }

        //ROL - Zero
        case 0x26:{
            uint8_t address = read_mem(program_counter);
            uint8_t value = read_mem(address);


            uint8_t old_c = get_flag(FLAG_C);
            set_flag(FLAG_C, value & 0x80);
            value = value << 1;
            if (old_c == 1){
                value |= 0x1;
            }
            set_zn(value);
            write_mem(address, value);

            cycles+=5;
            program_counter+=1;
            break;
        }

        //JSR
        case 0x20:{
            uint8_t low_byte = read_mem(program_counter);
            uint8_t high_byte = read_mem(program_counter+1);
            uint16_t address = (high_byte << 8) | low_byte;

            push((program_counter+1) >> 8);
            push((program_counter+1) & 0xFF);

            cycles += 6;

            program_counter = address;
            break;
        }

        //JMP
        case 0x4C:{
            uint8_t low_byte = read_mem(program_counter);
            uint8_t high_byte = read_mem(program_counter+1);
            uint16_t address = (high_byte << 8) | low_byte;
            cycles += 3;

            program_counter = address;
            break;
        }

        //LDX
        case 0xA2: {
            x_register = read_mem(program_counter);
            program_counter+=1;
            cycles += 2;
            set_zn(x_register);

            break;
        }

        //LDX - ZERO
        case 0xA6: {
            x_register = read_mem(read_mem(program_counter));
            program_counter+=1;
            cycles += 3;
            set_zn(x_register);

            break;
        }

        // LDX - ABSOLUTE
        case 0xAE:{
            x_register = read_mem((read_mem(program_counter+1) << 8) | read_mem(program_counter));
            set_zn(x_register);

            program_counter+=2;
            cycles += 4;
            break;
        }

        //LDA
        case 0xA9:{
            accumulator = read_mem(program_counter);
            program_counter+=1;
            cycles += 2;
            set_zn(accumulator);

         
            break;
        }


        //LDA - Aboslute X
        case 0xBD:{
            uint16_t base = ((read_mem(program_counter+1) << 8) | read_mem(program_counter));
            accumulator = read_mem(base + x_register);
            program_counter+=2;
            cycles += 4;

            if ((base & 0xFF00) != ((base + x_register) & 0xFF00)){
                cycles += 1;
            }
            set_zn(accumulator);

            break;
        }


        //LDA - Inderict X
        case 0xA1:{
            uint8_t ptr = (read_mem(program_counter) + x_register) & 0xFF;
            uint16_t address = ((read_mem((ptr+1)&0xFF) << 8) | read_mem(ptr));
            accumulator = read_mem(address);

            program_counter+=1;
            cycles += 6;
            set_zn(accumulator);
            break;
        }

        //LDA - Zero Page
        case 0xA5:{
            accumulator = read_mem(read_mem(program_counter));
            program_counter+=1;
            cycles += 3;
            set_zn(accumulator);

            break;
        }

        //LDA - Absolute
        case 0xAD:{
            accumulator = read_mem((read_mem(program_counter+1) << 8) | read_mem(program_counter));
            set_zn(accumulator);

            program_counter+=2;
            cycles += 4;
            break;
        }

        //LDY
        case 0xA0:{
            y_register = read_mem(program_counter);
            set_zn(y_register);

            program_counter+=1;
            cycles+=2;
            break;
        }

        //LDY - Absolute
        case 0xAC:{
            y_register = read_mem((read_mem(program_counter+1) << 8) | read_mem(program_counter));
            set_zn(y_register);

            program_counter+=2;
            cycles+=4;
            break;
        }

        //LDY - Zero page
        case 0xA4:{
            y_register = read_mem(read_mem(program_counter));
            set_zn(y_register);

            program_counter+=1;
            cycles+=3;
            break;
        }

        //STX
        case 0x86:{
            write_mem(read_mem(program_counter), x_register);
            program_counter+=1;
            cycles += 3;

            break;
        }

        //STY
        case 0x84:{
            write_mem(read_mem(program_counter), y_register);
            program_counter+=1;
            cycles += 3;

            break;
        }

        //STY - ABsolute
        case 0x8C:{
            write_mem((read_mem(program_counter+1) << 8) | read_mem(program_counter), y_register);
            program_counter+=2;
            cycles += 4;

            break;
        }


        // STX - ABSOLUTE
        case 0x8E:{
            write_mem((read_mem(program_counter+1) << 8) | read_mem(program_counter), x_register);

            program_counter+=2;
            cycles += 4;
            break;
        }

        //STA
        case 0x85:{
            write_mem(read_mem(program_counter), accumulator);
            program_counter+=1;
            cycles += 3;

            break;
        }

        //STA - Inderect Y
        case 0x91:{
            uint8_t ptr = read_mem(program_counter);
            uint16_t base = ((read_mem((ptr+1)&0xFF) << 8) | read_mem(ptr));
            uint16_t address = base + y_register;
            write_mem(address, accumulator);
            cycles+=6;
            program_counter+=1;
            break;
        }


        //STA - Inderect X
        case 0x81:{
            uint8_t ptr = (read_mem(program_counter) + x_register) & 0xFF;
            uint16_t address = ((read_mem((ptr+1)&0xFF) << 8) | read_mem(ptr));
            write_mem(address, accumulator);

            cycles+=6;
            program_counter+=1;
            break;
        }

        //STA - Absolute
        case 0x8D:{
            write_mem((read_mem(program_counter+1) << 8) | read_mem(program_counter), accumulator);
            program_counter+=2;
            cycles += 4;

            break;
        }

        //STA - Absolute Y
        case 0x99:{
            uint16_t base = ((read_mem(program_counter+1) << 8) | read_mem(program_counter));
            write_mem(base + y_register, accumulator);

            program_counter+=2;
            cycles += 5;

            break;
        }

        //SEI
        case 0x78:{
            cycles+=2;
            set_flag(FLAG_I, true);
            break;
        }

        //SED
        case 0xF8:{
            cycles+=2;
            set_flag(FLAG_D, true);
            break;
        }

        //PHP
        case 0x08:{
            push(proccesor_status_register | 0x30);
            cycles +=3;
            break;
        }

        //PHA
        case 0x48:{
            push(accumulator);
            cycles+=3;
            break;
        }

        //PLP
        case 0x28:{
            uint8_t proccesor_status = pull();

            set_flag(FLAG_N, proccesor_status & 0x80);                
            set_flag(FLAG_V, proccesor_status & 0x40);                
            set_flag(FLAG_D, proccesor_status & 0x8);                
            set_flag(FLAG_I, proccesor_status & 0x4);                
            set_flag(FLAG_Z, proccesor_status & 0x2);                
            set_flag(FLAG_C, proccesor_status & 0x1);                

            cycles +=4;
            break;
        }

        //EOR
        case 0x49:{
            accumulator = accumulator ^ read_mem(program_counter);
            set_zn(accumulator);

            program_counter+=1;
            cycles+=2;
            break;
        }

        //ADC
        case 0x69:{
            uint8_t value = read_mem(program_counter);
            uint16_t result = accumulator + value + get_flag(FLAG_C);
            
            set_flag(FLAG_C, result > 0xFF);
            
            if ((result ^ accumulator) & (result ^ value) & 0x80){
                set_flag(FLAG_V, true);
            }else{
                set_flag(FLAG_V, false);
            }


            set_zn((uint8_t)result);

            accumulator = (uint8_t)result;

            program_counter+=1;
            cycles+=2;
            break;
        }

        //ADC - ZEROPAGE
        case 0x65:{
            uint8_t value = read_mem(read_mem(program_counter));
            uint16_t result = accumulator + value + get_flag(FLAG_C);
            
            set_flag(FLAG_C, result > 0xFF);
            
            if ((result ^ accumulator) & (result ^ value) & 0x80){
                set_flag(FLAG_V, true);
            }else{
                set_flag(FLAG_V, false);
            }


            set_zn((uint8_t)result);

            accumulator = (uint8_t)result;

            program_counter+=1;
            cycles+=3;
            break;
        }

        case 0X61:{
            uint8_t ptr = (read_mem(program_counter) + x_register) & 0xFF;
            uint16_t address = ((read_mem((ptr+1)&0xFF) << 8) | read_mem(ptr));

            uint8_t value = read_mem(address);
            uint16_t result = accumulator + value + get_flag(FLAG_C);
            
            set_flag(FLAG_C, result > 0xFF);
            
            if ((result ^ accumulator) & (result ^ value) & 0x80){
                set_flag(FLAG_V, true);
            }else{
                set_flag(FLAG_V, false);
            }


            set_zn((uint8_t)result);

            accumulator = (uint8_t)result;

            program_counter+=1;
            cycles+=6;
            break;
        }

        //SBC
        case 0xE9:{
            uint8_t value = read_mem(program_counter);
            uint8_t inv = value ^ 0xFF;
            uint16_t result = accumulator + inv + get_flag(FLAG_C);
            
            set_flag(FLAG_C, (result > 0xFF));
            
            if ((result ^ accumulator) & (result ^ inv) & 0x80){
                set_flag(FLAG_V, true);
            }else{
                set_flag(FLAG_V, false);
            }


            set_zn((uint8_t)result);

            accumulator = (uint8_t)result;

            program_counter+=1;
            cycles+=2;
            break;
        }

        //SBC - ZERO
        case 0xE5:{
            uint8_t value = read_mem(read_mem(program_counter));
            uint8_t inv = value ^ 0xFF;
            uint16_t result = accumulator + inv + get_flag(FLAG_C);
            
            set_flag(FLAG_C, (result > 0xFF));
            
            if ((result ^ accumulator) & (result ^ inv) & 0x80){
                set_flag(FLAG_V, true);
            }else{
                set_flag(FLAG_V, false);
            }


            set_zn((uint8_t)result);

            accumulator = (uint8_t)result;

            program_counter+=1;
            cycles+=3;
            break;
        }

        //SBC - IX
        case 0xE1:{
            uint8_t ptr = (read_mem(program_counter) + x_register) & 0xFF;
            uint16_t address = ((read_mem((ptr+1)&0xFF) << 8) | read_mem(ptr));

            uint8_t value = read_mem(address);
            uint8_t inv = value ^ 0xFF;
            uint16_t result = accumulator + inv + get_flag(FLAG_C);
            
            set_flag(FLAG_C, (result > 0xFF));
            
            if ((result ^ accumulator) & (result ^ inv) & 0x80){
                set_flag(FLAG_V, true);
            }else{
                set_flag(FLAG_V, false);
            }


            set_zn((uint8_t)result);

            accumulator = (uint8_t)result;

            program_counter+=1;
            cycles+=6;
            break;
        }

        //INY
        case 0xC8:{
            y_register += 1;
            set_zn(y_register);
            cycles+=2;
            break;
        }


        //INX
        case 0xE8:{
            x_register += 1;
            set_zn(x_register);
            cycles+=2;
            break;
        }

        //DEY
        case 0x88:{
            y_register -= 1;
            set_zn(y_register);
            cycles+=2;
            break;
        }


        //DEX
        case 0xCA:{
            x_register -= 1;
            set_zn(x_register);
            cycles+=2;
            break;
        }

        //TAY
        case 0xA8:{
            y_register = accumulator;
            set_zn(y_register);
            cycles+=2;
            break;
        }

        //TXS
        case 0x9A:{
            stack_pointer = x_register;
            cycles+=2;
            break;
        }

        //TYA
        case 0x98:{
            accumulator = y_register;
            set_zn(accumulator);
            cycles+=2;
            break;
        }

        //TXA
        case 0x8A:{
            accumulator = x_register;
            set_zn(accumulator);
            cycles+=2;
            break;
        }


        //TSX
        case 0xBA:{
            x_register = stack_pointer;
            set_zn(x_register);
            cycles+=2;
            break;
        }

        //TAX
        case 0xAA:{
            x_register = accumulator;
            set_zn(x_register);
            cycles+=2;
            break;
        }

        //CLV
        case 0xB8:{
            set_flag(FLAG_V, false);
            cycles+=2;
            break;
        }

        //ORA
        case 0x09:{
            accumulator = accumulator | read_mem(program_counter);
            set_zn(accumulator);

            program_counter+=1;
            cycles+=2;
            break;
        }

        //ORA - ZERO
        case 0x05:{
            accumulator = accumulator | read_mem(read_mem(program_counter));
            set_zn(accumulator);

            program_counter+=1;
            cycles+=3;
            break;
        }

        //ORA - Inderict X
        case 0x01:{
            uint8_t ptr = (read_mem(program_counter) + x_register) & 0xFF;
            uint16_t address = ((read_mem((ptr+1)&0xFF) << 8) | read_mem(ptr));

            accumulator = accumulator | read_mem(address);
            set_zn(accumulator);

            program_counter+=1;
            cycles+=6;
            break;
        }


        //PLA
        case 0x68:{
            accumulator = pull();
            set_zn(accumulator);
            cycles+=4;
            break;
        }

        //AND
        case 0x29:{
            accumulator &= read_mem(program_counter);

            set_zn(accumulator);

            program_counter+=1;
            cycles+=2;
            break;
        }


        //AND - ZERO    
        case 0x25:{
            accumulator &= read_mem(read_mem(program_counter));

            set_zn(accumulator);

            program_counter+=1;
            cycles+=3;
            break;
        }

        //AND - Inderict X
        case 0x21:{
            uint8_t ptr = (read_mem(program_counter) + x_register) & 0xFF;
            uint16_t address = ((read_mem((ptr+1)&0xFF) << 8) | read_mem(ptr));

            accumulator &= read_mem(address);

            set_zn(accumulator);

            program_counter+=1;
            cycles+=6;
            break;
        }


        //EOR - ZERO
        case 0x45:{
            accumulator ^= read_mem(read_mem(program_counter));

            set_zn(accumulator);

            program_counter+=1;
            cycles+=3;
            break;
        }

        //EOR - Inderict X
        case 0x41:{
            uint8_t ptr = (read_mem(program_counter) + x_register) & 0xFF;
            uint16_t address = ((read_mem((ptr+1)&0xFF) << 8) | read_mem(ptr));

            accumulator ^= read_mem(address);

            set_zn(accumulator);

            program_counter+=1;
            cycles+=6;
            break;
        }

        //CMP
        case 0xC9:{
            uint8_t value = read_mem(program_counter);
            set_flag(FLAG_C, accumulator >= value);
            set_flag(FLAG_Z, accumulator == value);
            set_flag(FLAG_N, (uint8_t)(accumulator-value) & 0x80);

            cycles+=2;
            program_counter+=1;
            break;
        }

        //CMP - ZERO
        case 0xC5:{
            uint8_t value = read_mem(read_mem(program_counter));
            set_flag(FLAG_C, accumulator >= value);
            set_flag(FLAG_Z, accumulator == value);
            set_flag(FLAG_N, (uint8_t)(accumulator-value) & 0x80);

            cycles+=3;
            program_counter+=1;
            break;
        }


        //CMP - Inderict X
        case 0xC1:{
            uint8_t ptr = (read_mem(program_counter) + x_register) & 0xFF;
            uint16_t address = ((read_mem((ptr+1)&0xFF) << 8) | read_mem(ptr));

            uint8_t value = read_mem(address);
            set_flag(FLAG_C, accumulator >= value);
            set_flag(FLAG_Z, accumulator == value);
            set_flag(FLAG_N, (uint8_t)(accumulator-value) & 0x80);

            cycles+=6;
            program_counter+=1;
            break;
        }

        //CPY
        case 0xC0:{
            uint8_t value = read_mem(program_counter);
            set_flag(FLAG_C, y_register >= value);
            set_flag(FLAG_Z, y_register == value);
            set_flag(FLAG_N, (uint8_t)(y_register-value) & 0x80);

            cycles+=2;
            program_counter+=1;
            break;
        }

        //CPY - ZERO
        case 0xC4:{
            uint8_t value = read_mem(read_mem(program_counter));
            set_flag(FLAG_C, y_register >= value);
            set_flag(FLAG_Z, y_register == value);
            set_flag(FLAG_N, (uint8_t)(y_register-value) & 0x80);

            cycles+=3;
            program_counter+=1;
            break;
        }

        //CPX
        case 0xE0:{
            uint8_t value = read_mem(program_counter);
            set_flag(FLAG_C, x_register >= value);
            set_flag(FLAG_Z, x_register == value);
            set_flag(FLAG_N, (uint8_t)(x_register-value) & 0x80);

            cycles+=2;
            program_counter+=1;
            break;
        }

        //CPX - ZERO
        case 0xE4:{
            uint8_t value = read_mem(read_mem(program_counter));
            set_flag(FLAG_C, x_register >= value);
            set_flag(FLAG_Z, x_register == value);
            set_flag(FLAG_N, (uint8_t)(x_register-value) & 0x80);

            cycles+=3;
            program_counter+=1;
            break;
        }
        
        //CLD
        case 0xD8:{
            set_flag(FLAG_D, false);
            cycles+=2;
            break;
        }

        // INC
        case 0xE6:{
            uint8_t address = read_mem(program_counter);
            uint8_t value = read_mem(address);

            value +=1;
            set_zn(value);

            write_mem(address, value);

            cycles+=5;
            program_counter+=1;
            break;
        }

        // INC - Absolute
        case 0xEE:{
            uint16_t address = (read_mem(program_counter+1) << 8) | read_mem(program_counter);
            uint8_t value = read_mem((read_mem(program_counter+1) << 8) | read_mem(program_counter));

            value +=1;
            set_zn(value);

            write_mem(address, value);

            cycles+=6;
            program_counter+=2;
            break;
        }

        // DEC
        case 0xC6:{
            uint8_t address = read_mem(program_counter);
            uint8_t value = read_mem(address);

            value -=1;
            set_zn(value);

            write_mem(address, value);

            cycles+=5;
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


void cpu::nmi(){
    push(program_counter >> 8);
    push(program_counter & 0xFF);
    push((proccesor_status_register & ~0x10) | 0x20);  

    set_flag(FLAG_I, true);

    uint8_t low_byte = read_mem(0xFFFA);
    uint8_t high_byte = read_mem(0xFFFB);
    program_counter = (high_byte << 8) | low_byte;

    cycles += 7;
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
    cycles = 7;

    // set program counter to 0xc000 for nestest automating
    //program_counter = 0xC000;

    cpu_state = CPU_RUNNING;
}
