#include "chip8.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const uint8_t chip8_fontset[80] = 
{
    0xF0,0x90,0x90,0x90,0xF0, // 0
    0x20,0x60,0x20,0x20,0x70, // 1
    0xF0,0x10,0xF0,0x80,0xF0, // 2
    0xF0,0x10,0xF0,0x10,0xF0, // 3
    0x90,0x90,0xF0,0x10,0x10, // 4
    0xF0,0x80,0xF0,0x10,0xF0, // 5
    0xF0,0x80,0xF0,0x90,0xF0, // 6
    0xF0,0x10,0x20,0x40,0x40, // 7
    0xF0,0x90,0xF0,0x90,0xF0, // 8
    0xF0,0x90,0xF0,0x10,0xF0, // 9
    0xF0,0x90,0xF0,0x90,0x90, // A
    0xE0,0x90,0xE0,0x90,0xE0, // B
    0xF0,0x80,0x80,0x80,0xF0, // C
    0xE0,0x90,0x90,0x90,0xE0, // D
    0xF0,0x80,0xF0,0x80,0xF0, // E
    0xF0,0x80,0xF0,0x80,0x80  // F
};

// function to initialize the chip8 state
void chip8_init(Chip8 *chip8)
{
    // fills the entire struct with 0 bytes
    memset(chip8, 0, sizeof(Chip8));

    // set the program counter to 0x200 because programs are loaded
    // starting at address 0x200
    chip8->pc = 0x200;

    // load fontset into memory at 0x50
    memcpy(&chip8->memory[0x50], chip8_fontset, sizeof(chip8_fontset));
}


// function to load a rom file into chip8 memory. basically takes the game program and puts it into the RAM of our emulator
int chip8_load_rom(Chip8 *chip8, const char *filename)
{
    // open the file for reading binary
    FILE *file = fopen(filename, "rb");

    // variable to store the size of the ROM
    long size;

    // variable to store the amount of bytes copied
    size_t bytes_read;

    // make sure the file opened
    if (file == NULL)
    {
        perror("fopen");

        // return 0 for failure
        return 0;
    }

    // move to the end of the ROM file, find the size, 
    // and then rewind back to the beginning
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);

    // make sure the ROM file size is valid. 
    // cannot be bigger than the space alloted for the emulator
    if (size <= 0 || size > (4096 - 0x200))
    {
        fprintf(stderr, "Invalid ROM size\n");
        fclose(file);

        // return 0 for failure
        return 0;
    }

    // read (size) # of bytes from the rom file (file) one byte at a time into memory at 0x200
    bytes_read = fread(&chip8->memory[0x200], 1, size, file);

    // close the file
    fclose(file);

    // make sure the full ROM file was read
    if (bytes_read != (size_t)size)
    {
        fprintf(stderr, "Failed to read full ROM\n");

        // return 0 for failure
        return 0;
    }

    // return 1 for success
    return 1;

}

// function to read a 2-byte instruction from RAM and turn it into a 16-bit value
uint16_t chip8_fetch_opcode(Chip8 *chip8)
{
    // variables to store the high byte, low byte, and opcode for the instruction
    uint16_t high_byte;
    uint16_t low_byte;
    uint16_t opcode;

    // get the high byte at the pc, and the low byte by moving one byte over
    high_byte = chip8->memory[chip8->pc];
    low_byte = chip8->memory[chip8->pc + 1];

    // combine the high and low byte by shifting the high byte to make room for the low byte
    opcode = (high_byte << 8) | low_byte;

    // move the program counter
    chip8->pc += 2;

    // return the opcode
    return opcode;
}


// function to execute a given opcode. 
// basically stores the logic for the CPU behavior depending on the opcode
void chip8_execute_opcode(Chip8 *chip8, uint16_t opcode)
{
    // variables to store useful parts of the opcode
    uint8_t x;
    uint8_t nn;
    uint16_t nnn;

    // gets bits 9 - 12 for x, and keeps only those
    x = (opcode & 0x0F00) >> 8;

    // gets bits 1 - 8
    nn = opcode & 0x00FF;

    // gets bits 1 - 12
    nnn = opcode & 0x0FFF;

    // gets bits 13 - 16 for the instruction family
    switch (opcode & 0xF000)
    {
        // instructions starting with 0
        case 0x0000:
            // clear display instruction
            if (opcode == 0x00E0)
            {
                // set the display to all zeros, turning all pixels off
                memset(chip8->display, 0, sizeof(chip8->display));

                // lets the renderer know that the screen changed
                chip8->draw_flag = true;

            }
            else
            {
                fprintf(stderr, "Unknown opcode: 0x%04X\n", opcode);
                exit(1);
            }
            break;

        // instructions starting with 1, aka JUMP
        case 0x1000:
            // jump to address NNN
            chip8->pc = nnn;
            break;

        // instructions starting with 6, aka SET
        case 0x6000:
            // set the desired register to an address
            chip8->V[x] = nn;

            break;

        // instructions starting with 7, aka ADD
        case 0x7000:
            // add the address to a register
            chip8->V[x] += nn;

            break;

        // instructions starting with A, aka Index Register
        case 0xA000:
            // set the index register to an address
            chip8->I = nnn;

            break;

        default:
            fprintf(stderr, "Unknown opcode: 0x%04X\n", opcode);
            exit(1);
    }
}


// execute one cpu cycle
void chip8_cycle(Chip8 *chip8)
{
    // variable to store the opcode
    uint16_t opcode;

    // fetch the opcode
    opcode = chip8_fetch_opcode(chip8);

    // debug info to see what the emulator is doing
    printf("PC: 0x%03X  OPCODE: 0x%04X\n", chip8->pc, opcode);

    // execute the opcode
    chip8_execute_opcode(chip8, opcode);
}
