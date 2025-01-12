#include "chip8.h"

Chip8::Chip8() {
    for (unsigned char& i : RAM) {
        i = 0;
    }
    for (int i = 0; i < 80; i++) {
        RAM[0x50 + i] = FONT[i];
    }
    for (bool& i : FRAMEBUFFER) {
        i = false;
    }

    SP = 0;
    PC = 0x200;
    cycles = 0;
}

bool Chip8::LoadRom(const char* filename) {
    // Open the file as a stream of binary and move the file pointer to the end
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (file.is_open())
    {
        // Get size of file and allocate a buffer to hold the contents
        std::streampos size = file.tellg();
        char* buffer = new char[size];

        // Go back to the beginning of the file and fill the buffer
        file.seekg(0, std::ios::beg);
        file.read(buffer, size);
        file.close();

        // Load the ROM contents into the Chip8's memory, starting at 0x200
        for (long i = 0; i < size; ++i)
        {
            RAM[0x200 + i] = buffer[i];
        }

        // Free the buffer
        delete[] buffer;

        return true;
    }

    return false;
}

void Chip8::OneCycle() {
    if (PC >= 4096) {
        std::cout << "PC is too big" << std::endl;
        return;
    }
    const uint16_t opcode = RAM[PC] << 8 | (RAM[(PC + 1)]);
    if (!jf) {
        PC += 2;
    } else {
        jf = false;
    }

    if ((opcode & 0xF000) == 0x0) {
        switch (opcode) {
            //Clear screen
            case 0x00E0: {
                for (bool& i : FRAMEBUFFER) {
                    i = false;
                }
                break;
            }
            case 0x00EE: {
                SP--;
                PC = STACK[SP];
                STACK[SP] = 0x0;
                break;
            }
            default: {
                if (!opcode) {
                    break;
                }
                std::cout << "Unknown opcode: " << opcode << std::endl;
                break;
            }
        }
    } else if ((opcode & 0xF000) >> 12 == 0x8) {
        switch (opcode & 0xF00F) {
            //Set
            case 0x8000: {
                V[THIRDNIB(opcode)] = V[SECONDNIB(opcode)];
                break;
            }
            //Or
            case 0x8001: {
                V[THIRDNIB(opcode)] = V[THIRDNIB(opcode)] | V[SECONDNIB(opcode)];
                break;
            }
            //And
            case 0x8002: {
                V[THIRDNIB(opcode)] = V[THIRDNIB(opcode)] & V[SECONDNIB(opcode)];
                break;
            }
            //Xor
            case 0x8003: {
                V[THIRDNIB(opcode)] = V[THIRDNIB(opcode)] ^ V[SECONDNIB(opcode)];
                break;
            }
            //Add
            case 0x8004: {
                V[THIRDNIB(opcode)] = V[THIRDNIB(opcode)] + V[SECONDNIB(opcode)];
                break;
            }
            //Sub third from second
            case 0x8005: {
                V[THIRDNIB(opcode)] = V[THIRDNIB(opcode)] - V[SECONDNIB(opcode)];
                if (V[THIRDNIB(opcode)] > V[SECONDNIB(opcode)]) {
                    V[0xF] = 1;
                } else {
                    V[0xF] = 0;
                }
                break;
            }
            //Shift to the right
            case 0x8006: {
                if (cosmac)
                    V[THIRDNIB(opcode)] = V[SECONDNIB(opcode)];
                V[0xF] = V[THIRDNIB(opcode)] & 0b00000001;
                V[THIRDNIB(opcode)] = V[THIRDNIB(opcode)] >> 1;
                break;
            }
            //Sub second from third
            case 0x8007: {
                V[THIRDNIB(opcode)] = V[SECONDNIB(opcode)] - V[THIRDNIB(opcode)];
                if (V[SECONDNIB(opcode)] > V[THIRDNIB(opcode)]) {
                    V[0xF] = 1;
                } else {
                    V[0xF] = 0;
                }
                break;
            }
            //Shift to the left
            case 0x800E: {
                if (cosmac)
                    V[THIRDNIB(opcode)] = V[SECONDNIB(opcode)];
                V[0xF] = V[THIRDNIB(opcode)] & 0b10000000;
                V[THIRDNIB(opcode)] = V[THIRDNIB(opcode)] << 1;
                break;
            }
        }
    } else if ((opcode & 0xF000) >> 12 == 0xF) {
            switch (opcode * 0xF0FF) {
                case 0xF055: {
                    if (cosmac) {

                    }
                }
            }
    } else {
        switch (opcode & 0xF000) {
            //Jump
            case 0x1000: {
                PC = opcode & 0x0FFF;
                jf = true;
                break;
            }
            case 0x2000: {
                uint16_t addr = opcode & 0x0FFF;
                STACK[SP] = PC;
                SP++;
                PC = addr;
                jf = true;
                break;
            }
            //Skip next instruction if equal
            case 0x3000: {
                if (V[THIRDNIB(opcode)] == (opcode & 0x00FF))
                    PC += 2;
                break;
            }
            //Skip next instruction if not equal
            case 0x4000: {
                if (V[THIRDNIB(opcode)] != (opcode & 0x00FF))
                    PC += 2;
                break;
            }
            //Skip next instruction if equal
            case 0x5000: {
                if (V[THIRDNIB(opcode)] == V[SECONDNIB(opcode)])
                    PC += 2;
                break;
            }
            //Set register VX
            case 0x6000: {
                V[THIRDNIB(opcode)] = opcode & 0x00FF;
                break;
            }
            //Add value to register VX
            case 0x7000: {
                V[THIRDNIB(opcode)] += opcode & 0x00FF;
                break;
            }
            //Skip next instruction if not equal
            case 0x9000: {
                if (V[THIRDNIB(opcode)] != V[SECONDNIB(opcode)])
                    PC += 2;
                break;
            }
            //Set index register
            case 0xA000: {
                I = opcode & 0x0FFF;
                break;
            }
            case 0xB000: {
                if (cosmac) {
                    PC = (opcode & 0x0FFF) + V[0];
                } else {
                    PC = (opcode & 0x0FFF) + V[THIRDNIB(opcode)];
                }
                break;
            }
            //Draw framebuffer
            case 0xD000: {
                uint8_t x = V[THIRDNIB(opcode)] & 63;
                uint8_t y = V[SECONDNIB(opcode)] & 31;
                V[0xF] = 0;
                uint8_t n = FIRSTNIB(opcode);

                for (int row = 0; row < n; row++) {
                    uint8_t nbyte = RAM[I + row];
                    for (int col = 0; col < 8; col++) {
                        uint8_t bit = nbyte & (0x80u >> col);
                        if (bit) {
                            if (FRAMEBUFFER[(x + col) + width * (y + row)]) {
                                V[0xF] = 1;
                            }
                            FRAMEBUFFER[(x + col) + width * (y + row)] ^= true;
                        }
                    }
                }
                break;
            }
            default: {
                std::cout << "Unknown opcode: " << std::hex << opcode << std::dec << std::endl;
                return;
            }
        }
    }

    cycles++;
}
