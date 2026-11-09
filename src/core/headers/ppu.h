#pragma once

#include "core/headers/ppu_bus.h"
#include <cstdint>

class ppu{
    private:

    ppu_bus *m_bus = nullptr;
    uint8_t pattern[128 * 128] = {};

    uint8_t ctrl = 0;          // $2000
    uint8_t mask = 0;          // $2001
    bool v_blank = false;    // $2002 bit 7
    bool sprite0_hit = false;     // $2002 bit 6
    bool sprite_overflow = false; // $2002 bit 5
    uint8_t oam_addr = 0;      // $2003
    uint8_t oam[256] = {};     // read/written via $2004
    uint16_t vram_addr = 0;     // set by two writes to $2006
    bool w = false;         // which of the two writes comes next ($2005/$2006)
    uint8_t read_buffer = 0;   // delayed result for $2007 reads

    uint8_t scroll_x = 0;
    uint8_t scroll_y = 0;


    uint16_t dot = 0;
    uint16_t scanline = 0;

    public:
    
    const uint8_t* get_pattern_table(int table);
    
    bool frame_complete = false;
    bool nmi_pending = false;

    ppu();

    bool take_nmi();

    void load_cartridge(cartridge *cart);

    uint8_t cpu_read(uint8_t reg);
    void cpu_write(uint8_t reg, uint8_t val);

    void step();

};