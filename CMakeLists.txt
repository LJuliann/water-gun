# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.1)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.21)
# Configuration initiale pour le RP2350
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Utiliser le SDK local si aucun chemin explicite n'est fourni.
if(NOT PICO_SDK_PATH AND NOT DEFINED ENV{PICO_SDK_PATH}
   AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/.tools/pico-sdk/pico_sdk_init.cmake")
    set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/.tools/pico-sdk")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
