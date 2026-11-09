#pragma once

#include <cstdint>
#include "bus.h"
#include "core/headers/cartridge.h"


// CPU STATES
const int CPU_IDLE = 0;
const int CPU_RUNNING = 1;
const int CPU_PAUSED = 2;
const int CPU_CRASHED = 3;

// FLAG MASKS
const uint8_t FLAG_C = 0x01;
const uint8_t FLAG_Z = 0x02;
const uint8_t FLAG_I = 0x04;
const uint8_t FLAG_D = 0x08;
const uint8_t FLAG_B = 0x10;
const uint8_t FLAG_UNUSED = 0x20;
const uint8_t FLAG_V = 0x40;
const uint8_t FLAG_N = 0x80;

class cpu{
    private:

    /*
    0x0:0x07FF - 2kb internal memory
    0x0800:0x0FFF - Mirror of memory 1
    0x1000:17FF - Mirror of memory 2
    0x1800:1FFF - Mirror of memory 3
    0x2000:0x2007 - PPU registers
    0x2008:0x3FFF - Mirrors PPU registers (repeat every 8 Bytes)
    0x4000:0x4017 - Nes APU and I/O Registers
    0x4018:0x401F - APU and I/O functionality that is normally disabled
    0x4020:0xFFFF - Unmapped used for cartridges
    0x6000:0x7FFF - Usually cartridge ram
    0x8000:0xFFFF - Usually rom and mappers
    */

    uint16_t program_counter = 0;
    uint8_t accumulator = 0;
    uint8_t x_register = 0;
    uint8_t y_register = 0;
    uint8_t stack_pointer = 0;
    bus *m_bus = nullptr;

    /*
    Bit 7 — Negative (N): Set to 1 if the high bit (bit 7) of the result from the last operation is set (indicating a negative number in signed arithmetic).
    Bit 6 — Overflow (V): Set to 1 when a signed two's complement arithmetic overflow occurs.
    Bit 5 — Unused / Reserved (-): Always hard-coded to 1.
    Bit 4 — Break (B): Set to 1 when a software BRK instruction triggers an interrupt.
    Bit 3 — Decimal (D): When set to 1, puts the ALU into Binary Coded Decimal (BCD) mode for arithmetic.
    Bit 2 — Interrupt Disable (I): When set to 1, masks (ignores) standard hardware maskable interrupts (IRQ).
    Bit 1 — Zero (Z): Set to 1 when the result of the last operation is equal to zero.
    Bit 0 — Carry (C): Set to 1 if an arithmetic operation produces a carry-out or a borrow (or during shift/rotate instructions).
    */

    uint8_t proccesor_status_register = 0;

    int cpu_state = 0;
    uint64_t cycles = 0;

    void set_flag(uint8_t mask, bool val);
    bool get_flag(uint8_t mask);
    void set_zn(uint8_t val);

    void branch(bool flag);

    void push(uint8_t val);
    uint8_t pull();

    void opcode_message();
    void log();

    public:

    int get_cpu_state();

    cpu();

    void load_cartridge(cartridge *cart);

    void write_mem(uint16_t addr, uint8_t val);

    uint8_t read_mem(uint16_t addr);

    void power_on();
    void cycle();

};