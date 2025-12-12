# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
set(PICO_BOARD board)
list(APPEND PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_LIST_DIR})

include(pico_sdk_import.cmake)
project(ape-audio-synthesizer C CXX ASM)

pico_sdk_init()

# Super important for u8g2 to prune unused fonts (otherwise 11 MB binary :skull:)
add_compile_options(-ffast-math -ffunction-sections -fdata-sections)
add_link_options(-Wl,--gc-sections)

add_subdirectory(lib/u8g2)

add_executable(ape-audio-synthesizer 
    src/main.c
    src/hal/display.c
    src/hal/encoders.c
)

pico_set_program_name(ape-audio-synthesizer "ape-audio-synthesizer")
pico_set_program_version(ape-audio-synthesizer "0.1")

pico_enable_stdio_uart(ape-audio-synthesizer 0)
pico_enable_stdio_usb(ape-audio-synthesizer 1)

target_link_libraries(ape-audio-synthesizer
    u8g2 
    m
    hardware_pio
    hardware_spi
    pico_stdlib
)

target_include_directories(ape-audio-synthesizer PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
)
pico_generate_pio_header(ape-audio-synthesizer ${CMAKE_CURRENT_LIST_DIR}/src/hal/encoders.pio)

pico_add_extra_outputs(ape-audio-synthesizer)
