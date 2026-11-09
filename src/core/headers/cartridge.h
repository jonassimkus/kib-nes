#pragma  once

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <vector>

class cartridge{
    private:

    std::vector<uint8_t> trainer = std::vector<uint8_t>();
    std::vector<uint8_t> header = std::vector<uint8_t>();

    int pgr_size = 0;
    int chr_size = 0;

    std::vector<uint8_t> pgr = std::vector<uint8_t>();
    std::vector<uint8_t> pgr_ram = std::vector<uint8_t>();
    std::vector<uint8_t> chr = std::vector<uint8_t>();

    std::vector<uint8_t> cache = std::vector<uint8_t>();

    public:

    void append_cache(uint8_t value){
        cache.push_back(value);
    }

    void onload();
    void unload();

    uint8_t get_pgr_size(){
        return pgr_size;
    }

    uint8_t get_chr_size(){
        return chr_size;
    }

    uint8_t read_chr(uint16_t addr);
    void write_chr(uint16_t addr, uint8_t val);
    
    uint8_t read_mem(uint16_t addr);
    void write_mem(uint16_t addr, uint8_t val);

    bool is_valid_rom();

    void print_all_memory(){
        for (int i = 0; i < pgr.size(); ++i){
            printf("%04X\n", pgr[i]);
        }

        std::cout << "Total size " << (pgr.size() + header.size() + chr.size() + trainer.size()) << " Bytes" << std::endl;
    }

};