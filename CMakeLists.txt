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
if (EXISTS ${picoVscode} AND NOT PICO_SDK_PATH AND NOT DEFINED ENV{PICO_SDK_PATH})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.21)
# Carte par défaut : Pico 2 (RP2350). Le preset pico-usb choisit la Pico 1 (RP2040).
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Utiliser le SDK local si aucun chemin explicite n'est fourni.
if(NOT PICO_SDK_PATH AND NOT DEFINED ENV{PICO_SDK_PATH}
   AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/.tools/pico-sdk/pico_sdk_init.cmake")
    set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/.tools/pico-sdk")
endif()

# Privilégier les outils locaux lorsqu'aucune installation n'est imposée.
if(NOT PICO_TOOLCHAIN_PATH AND NOT DEFINED ENV{PICO_TOOLCHAIN_PATH}
   AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/.tools/arm-toolchain/bin/arm-none-eabi-gcc")
    set(PICO_TOOLCHAIN_PATH "${CMAKE_CURRENT_LIST_DIR}/.tools/arm-toolchain" CACHE PATH "Arm toolchain")
endif()
if(NOT picotool_DIR
   AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/.tools/picotool/picotoolConfig.cmake")
    set(picotool_DIR "${CMAKE_CURRENT_LIST_DIR}/.tools/picotool" CACHE PATH "picotool package")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(pico_sdk_import.cmake)

project(
water_gun
VERSION 1 
LANGUAGES C CXX ASM
)

pico_sdk_init()

# FreeRTOS : port choisi selon la puce de la carte.
set(FREERTOS_KERNEL_PATH
    "${CMAKE_CURRENT_LIST_DIR}/FreeRTOS-Kernel"
)
set(FREERTOS_CONFIG_FILE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")

if(PICO_PLATFORM STREQUAL "rp2040")
    set(FREERTOS_PORT RP2040)
elseif(PICO_PLATFORM STREQUAL "rp2350-arm-s")
    set(FREERTOS_PORT RP2350_ARM_NTZ)
else()
    message(FATAL_ERROR "Plateforme non prise en charge : ${PICO_PLATFORM}")
endif()
include(
    "${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/${FREERTOS_PORT}/FreeRTOS_Kernel_import.cmake"
)

add_executable(water_gun
    water_gun.c
    gun_utils.c
)

option(WATER_GUN_USE_UART_IRQ "Receive commands using UART0 RX interrupts" OFF)
option(WATER_GUN_DEBUG "Enable firmware diagnostic prints" OFF)
option(WATER_GUN_STDIO_USB "Receive commands over USB CDC instead of UART0" OFF)
if(WATER_GUN_STDIO_USB AND WATER_GUN_USE_UART_IRQ)
    message(FATAL_ERROR "WATER_GUN_STDIO_USB et WATER_GUN_USE_UART_IRQ sont incompatibles")
endif()
target_compile_definitions(water_gun PRIVATE
    DEBUG=$<BOOL:${WATER_GUN_DEBUG}>
    USE_UART_IRQ=$<BOOL:${WATER_GUN_USE_UART_IRQ}>
)

target_include_directories(water_gun PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(water_gun
    pico_stdlib
    hardware_pwm
    FreeRTOS-Kernel-Heap4
)

if(WATER_GUN_USE_UART_IRQ)
    target_link_libraries(water_gun hardware_uart hardware_irq)
endif()

# USB CDC (/dev/ttyACM0 côté Jetson) ou UART0 sur GP0/GP1 (moniteur série Wokwi).
if(WATER_GUN_STDIO_USB)
    pico_enable_stdio_usb(water_gun 1)
    pico_enable_stdio_uart(water_gun 0)
else()
    pico_enable_stdio_usb(water_gun 0)
    pico_enable_stdio_uart(water_gun 1)
endif()

pico_add_extra_outputs(water_gun)


