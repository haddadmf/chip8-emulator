#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdbool.h>

// creating the virtual chip-8 cpu
typedef struct Chip8
{
    // ram for the chip8
    uint8_t memory[4096];

    // registers for the chip8 (V0 - VF)
    uint8_t V[16];

    // index register
    uint16_t I;

    // program counter
    uint16_t pc;

    // stack and stack pointer
    uint16_t stack[16];
    uint8_t sp;

    // timers
    uint8_t delay_timer;
    uint8_t sound_timer;

    // display and input keys
    uint8_t display[64 * 32];
    uint8_t keypad[16];

    // tells the main loop to redraw
    bool draw_flag;

} Chip8;

// helper functions for the emulator
void chip8_init(Chip8 *chip8);
int chip8_load_rom(Chip8 *chip8, const char *filename);
uint16_t chip8_fetch_opcode(Chip8 *chip8);
void chip8_execute_opcode(Chip8 *chip8, uint16_t opcode);
void chip8_cycle(Chip8 *chip8);

#endif