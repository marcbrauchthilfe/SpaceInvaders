# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico2-edu C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_executable(pico2-edu src/main.c
src/hal/displays/st7735.c
src/hal/controls/joystick.c
src/hal/leds/ws2812.c
src/hal/sensors/dht.c
src/hal/sensors/mpu6050.c
src/demos/display.c
src/demos/joystick.c
src/demos/leds.c
src/demos/dht11.c
src/demos/i2c_scan.c
src/demos/motion.c
src/demos/Abgabe_09.c
src/game/game.c
src/game/gamestate.c
)

pico_set_program_name(pico2-edu "pico2-edu")
pico_set_program_version(pico2-edu "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(pico2-edu 1)
pico_enable_stdio_usb(pico2-edu 0)

# Add the standard library to the build
target_link_libraries(pico2-edu
pico_stdlib
hardware_adc
hardware_spi
hardware_pio
hardware_dma
hardware_clocks
hardware_i2c
)

# Add the standard include files to the build
target_include_directories(pico2-edu PRIVATE
${CMAKE_CURRENT_LIST_DIR}
${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_definitions(pico2-edu PRIVATE
    PICO_DEFAULT_UART_TX_PIN=12
    PICO_DEFAULT_UART_RX_PIN=13
)

pico_add_extra_outputs(pico2-edu)

pico_generate_pio_header(pico2-edu ${CMAKE_CURRENT_LIST_DIR}/pio/ws2812.pio)
pico_generate_pio_header(pico2-edu ${CMAKE_CURRENT_LIST_DIR}/pio/dht.pio)