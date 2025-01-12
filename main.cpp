#include <iostream>

#include <SDL.h>

#include "chip8.h"

int main() {
    Chip8 chip8;

    if (!chip8.LoadRom("4-flags.ch8")) {
        std::cerr << "Failed to load rom" << std::endl;
        return -1;
    }

    std::string inp;
    std::cout << "Do you want to use the 1) COSMAC VIP instructions or 2) CHIP-48/SUPER-CHIP instructions (1/2): \n";
    std::cin >> inp;
    if (inp == "1") {
        chip8.cosmac = true;
        std::cout << "COSMAC VIP instructions chosen" << std::endl;
    } else {
        chip8.cosmac = false;
        std::cout << "CHIP-48/SUPER-CHIP instructions chosen" << std::endl;
    }

    SDL_Window* window = nullptr;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return -1;
    }
    window = SDL_CreateWindow(
        "Chip-8 emulator",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        1024, 512,
        SDL_WINDOW_SHOWN);

    if (window == nullptr) {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderSetLogicalSize(renderer, 1024, 512);

    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB888,
        SDL_TEXTUREACCESS_STREAMING,
        64, 32);

    uint32_t pixels[2048];

    while (true) {
        chip8.OneCycle();

        for (int i = 0; i < 2048; i++) {
            if (chip8.FRAMEBUFFER[i]) {
                pixels[i] = 0xFFFFFFFFFFFF;
            }
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exit(0);
            }
        }
        //Draw to screen (obviously update pixels)
        SDL_UpdateTexture(texture, nullptr, pixels, 64 * sizeof(uint32_t));

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }
}
