cmake_minimum_required(VERSION 3.17)
include(pico_sdk_import.cmake)
set(ENV{PICO_SDK_PATH} D:/Projects/pico/pico-sdk/)
set(PIMORONI_SDK D:/Projects/pico/pimoroni-pico/)
include(pimoroni_pico_display_import.cmake)

project(custom_display_cpp  C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_executable(custom_display_cpp main.cpp st7789.cpp st7789.h)
pico_sdk_init()

# Pull in our pico_stdlib which aggregates commonly used features
target_link_libraries(custom_display_cpp pico_stdlib pico_graphics hardware_spi hardware_pwm)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(custom_display_cpp)
