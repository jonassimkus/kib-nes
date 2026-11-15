#include "./headers/ppu.h"

ppu::ppu(){
    m_bus = new ppu_bus();
}

void ppu::load_cartridge(cartridge *cart){
    m_bus->load_cartridge(cart);
}

void ppu::cpu_write(uint8_t reg, uint8_t val){
    switch (reg){
        case 0: 
            ctrl = val;
            break;

        case 1:
            mask = val;
            break;

        case 3:
            oam_addr = val;
            break;

        case 4:
            oam[oam_addr] = val;
            oam_addr += 1;  
            break;

        case 5:
            if (!w){
                scroll_x = val;
            }else{
                scroll_y = val;
            }
            w = !w;
            break;

        case 6: 
            if (!w){
                vram_addr = ((val & 0x3F) << 8) | (vram_addr & 0x00FF);
            }else{
                vram_addr = (vram_addr & 0xFF00) | val;
            }
            w = !w;
            break;

        case 7: 
            m_bus->write_mem(vram_addr, val);
            vram_addr += (ctrl & 0x04) ? 32 : 1;
            break;

        default:
            break;
    }
}

uint8_t ppu::cpu_read(uint8_t reg){
    switch (reg){
        case 2:{
            uint8_t status = 0;
            if (v_blank){
                status |= 0x80;
            }
            if (sprite0_hit){
                status |= 0x40;
            }     
            if (sprite_overflow){
                status |= 0x20;
            }

            v_blank = false;  
            w = false; 
            return status;
        }

        case 4:{
            return oam[oam_addr];
        }

        case 7:{
            uint8_t value = read_buffer;
            read_buffer = m_bus->read_mem(vram_addr);

            if ((vram_addr & 0x3FFF) >= 0x3F00){
                value = read_buffer;
            }

            vram_addr += (ctrl & 0x04) ? 32 : 1;
            return value;
        }

        default:{
            return 0;
        }
    }
}

const uint8_t* ppu::get_pattern_table(int table){
    uint16_t base = table * 0x1000; 

    for (int tile = 0; tile < 256; ++tile){
        int tile_x = tile % 16;  
        int tile_y = tile / 16;

        for (int row = 0; row < 8; ++row){
            uint8_t plane0 = m_bus->read_mem(base + tile * 16 + row);
            uint8_t plane1 = m_bus->read_mem(base + tile * 16 + row + 8);

            for (int col = 0; col < 8; ++col){
                int bit = 7 - col;   
                uint8_t pixel = ((plane0 >> bit) & 1) | (((plane1 >> bit) & 1) << 1);

                int x = tile_x * 8 + col;
                int y = tile_y * 8 + row;
                pattern[y * 128 + x] = pixel; 
            }
        }
    }

    return pattern;
}

bool ppu::take_nmi(){
    bool pending = nmi_pending;
    nmi_pending = false;
    return pending;
}

void ppu::render_background(){
    uint8_t backdrop = m_bus->read_mem(0x3F00) & 0x3F;

    if (!(mask & 0x08)){
        for (int i = 0; i < 256 * 240; ++i){
            framebuffer[i] = backdrop;
        }
        return;
    }

    uint16_t nametable_base = 0x2000 + (ctrl & 0x03) * 0x400;
    uint16_t pattern_base   = (ctrl & 0x10) ? 0x1000 : 0x0000;

    for (int row = 0; row < 30; ++row){
        for (int col = 0; col < 32; ++col){
            uint8_t tile = m_bus->read_mem(nametable_base + row * 32 + col);

            uint8_t attribute = m_bus->read_mem(nametable_base + 0x3C0 + (row / 4) * 8 + (col / 4));
            int shift = ((row % 4) / 2) * 4 + ((col % 4) / 2) * 2;
            uint8_t palette = (attribute >> shift) & 0x03;

            for (int py = 0; py < 8; ++py){
                uint8_t plane0 = m_bus->read_mem(pattern_base + tile * 16 + py);
                uint8_t plane1 = m_bus->read_mem(pattern_base + tile * 16 + py + 8);

                for (int px = 0; px < 8; ++px){
                    int bit = 7 - px;
                    uint8_t value = ((plane0 >> bit) & 1) | (((plane1 >> bit) & 1) << 1);

                    uint8_t colour;
                    if (value == 0){
                        colour = backdrop;
                    }else{
                        colour = m_bus->read_mem(0x3F00 + palette * 4 + value) & 0x3F;
                    }

                    int x = col * 8 + px;
                    int y = row * 8 + py;
                    framebuffer[y * 256 + x] = colour;
                }
            }
        }
    }
}

void ppu::step(){
    dot += 1;
    if (dot > 340){
        dot = 0;
        scanline += 1;
        if (scanline > 261){
            scanline = 0;
            frame_complete = true;
        }
    }

    if (scanline == 241 && dot == 1){
        render_background();

        v_blank = true;
        if (ctrl & 0x80){
            nmi_pending = true;
        }
    }

    if (scanline == 261 && dot == 1){
        v_blank = false;
        sprite0_hit = false;
        sprite_overflow = false;
    }
}