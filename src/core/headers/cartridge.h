#pragma  once

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <vector>

class cartridge{
    private:

    std::vector<uint8_t> memory = std::vector<uint8_t>();

    public:

    void append_memory(uint8_t value){
        memory.push_back(value);
    }

    uint16_t pgr_size(){
        return memory[4];
    }

    uint16_t chr_size(){
        return memory[5];
    }

    uintptr_t read_mem(uint16_t addr){
        return memory[addr];
    }

    void print_all_memory(){
        for (int i = 0; i < 4; ++i){
            printf("%04X\n", memory[i]);
        }

        std::cout << "Total size " << memory.size() << " Bytes" << std::endl;

        printf("%04X\n", memory[4]);
        printf("%04X\n", memory[5]);

    }

};