cmake_minimum_required(VERSION 3.21)
# Configuration initiale pour le RP2350
set(PICO_BOARD pico2 CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(
water_gun
VERSION 1 
LANGUAGES C CXX ASM
)

pico_sdk_init()

add_executable(water_gun
water_gun.c
)

target_link_libraries(water_gun
pico_stdlib
)

pico_add_extra_outputs(water_gun)