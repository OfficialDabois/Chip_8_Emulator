cmake_minimum_required(VERSION 3.29)
project(chip_8_emu)

set(CMAKE_CXX_STANDARD 20)

find_package(SDL2 REQUIRED COMPONENTS SDL2)
add_executable(chip_8_emu main.cpp
        chip8.cpp
        chip8.h)
target_link_libraries(chip_8_emu PRIVATE SDL2::SDL2)
