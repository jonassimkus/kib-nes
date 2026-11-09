#include "./headers/cartridge.h"
#include <cstdint>

void cartridge::onload(){
    unload();

    if (cache.size() >= 16){
        pgr_size = cache[4];
        chr_size = cache[5];
    }

    if (!is_valid_rom()){
        std::cout << "Rom is invalid!" << std::endl;
        cache.clear();
        return;
    }else{
        std::cout << "Rom is valid!" << std::endl;
    }

    bool load_trainer = ((cache[6] >> 2) & 1) == 1;
    int current_offset = 0;

    for (int i = current_offset; i < 16; ++i){
        header.push_back(cache[i]);
    }

    current_offset+=16;

    if (load_trainer){
        for (int i = current_offset; i < 512 + current_offset; ++i){
            trainer.push_back(cache[i]);
        }
        current_offset += 512;
    }else{
        trainer.clear();
    }

    for (int i = current_offset; i < pgr_size * 16384 + current_offset; ++i){
        pgr.push_back(cache[i]);
    }

    current_offset += pgr_size * 16384;

    for (int i = current_offset; i < chr_size * 8192 + current_offset; ++i){
        chr.push_back(cache[i]);
    }

    if (chr_size == 0){
        for(int i = 0; i < 8192; ++i){
            chr.push_back(0);
        }
    }

    for (int i = 0; i < 8192; ++i){
        pgr_ram.push_back(0);
    }

    cache.clear();
    //print_all_memory();
}

void cartridge::unload(){
    trainer.clear();
    header.clear();
    pgr.clear();
    pgr_ram.clear();
    chr.clear();

    pgr_size = 0;
    chr_size = 0;
}

bool cartridge::is_vertical_mirroring(){
    return header[6] & 0x1;
}

uint8_t cartridge::read_chr(uint16_t addr){
    if (chr.size() == 0){
        return 0;
    }

    if (addr <= 0x1FFF){
        return chr[addr];
    }

    return 0;
}

void cartridge::write_chr(uint16_t addr, uint8_t val){
    if (chr_size != 0){
        return;
    }

    if (chr.empty()){
        return;
    }

    if (addr > 0x1FFF){
        return;
    }

    chr[addr] = val;
}

uint8_t cartridge::read_mem(uint16_t addr){
    if (pgr.size() == 0){
        return 0;
    }

    if (addr >= 0x8000 && addr <= 0xFFFF){
        return pgr[(addr - 0x8000) % pgr.size()];
    }
    
    if (addr >= 0x6000 && addr <= 0x7FFF){
        return pgr_ram[addr - 0x6000];
    }

    return 0;
}

void cartridge::write_mem(uint16_t addr, uint8_t val){
    if (pgr.size() == 0){
        return;
    }

    if (addr >= 0x6000 && addr <= 0x7FFF){
        pgr_ram[addr - 0x6000] = val;
    }
}

bool cartridge::is_valid_rom(){
    if (cache.size() < 16){
        return false;
    }

    if (cache[0] != 0x004E){
        return false;
    }
    if (cache[1] != 0x0045){
        return false;
    }
    if (cache[2] != 0x0053){
        return false;
    }
    if (cache[3] != 0x001A){
        return  false;
    }

    bool load_trainer = ((cache[6] >> 2) & 1) == 1;

    if (load_trainer && cache.size() < 512+16){
        return  false;
    }

    if (pgr_size == 0){
        return false;
    }

    if (pgr_size * 16384 > cache.size() - 16 - (512 * load_trainer)){
        return  false;
    }

    if (chr_size * 8192 > cache.size() - 16 - (512 * load_trainer) - (pgr_size * 16384)){
        return false;
    }
    
    return true;
}