#pragma  once

#include <cstdint>
class bus{
    private:
    
    uint8_t memory[2048] = {};

    public:

    void write_mem(uint16_t addr, uint8_t val);

    uint8_t read_mem(uint16_t addr);
};