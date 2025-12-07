cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
 
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(Diapasonix C CXX ASM)

add_definitions(-DBOARD_IS_PICO2)
add_definitions(-DPICO_PLATFORM=rp2350)
add_definitions(-DARDUINO_ARCH_RP2350=1)
add_definitions(-DARDUINO_ARCH_RP2040=1)  # Required for AMY library compatibility
set(PICO_BOARD pico2 CACHE STRING "Board type")

pico_sdk_init()

add_executable(${PROJECT_NAME}
        main.c
        multicore_audio.c
)

# Build AMY library manually, excluding i2s.c and amy_midi.c
add_library(amy INTERFACE)

target_sources(amy INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/lib/amy/amy/src/algorithms.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/amy/amy/src/amy.c
        # ${CMAKE_CURRENT_LIST_DIR}/lib/amy/amy/src/amy_midi.c  # EXCLUDED - we have our own
        ${CMAKE_CURRENT_LIST_DIR}/lib/amy/amy/src/api.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/amy/amy/src/custom.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/amy/amy/src/delay.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/amy/amy/src/envelope.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/amy/amy/src/examples.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/amy/amy/src/filters.c
        # ${CMAKE_CURRENT_LIST_DIR}/lib/amy/amy/src/i2s.c  # EXCLUDED - we have our own
        ${CMAKE_CURRENT_LIST_DIR}/lib/amy/amy/src/instrument.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/amy/amy/src/interp_partials.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/amy/amy/src/libminiaudio-audio.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/amy/amy/src/log2_exp2.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/amy/amy/src/midi_mappings.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/amy/amy/src/oscillators.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/amy/amy/src/parse.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/amy/amy/src/patches.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/amy/amy/src/pcm.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/amy/amy/src/pyamy.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/amy/amy/src/sequencer.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/amy/amy/src/transfer.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/amy/amy/src/usb.c
)

target_include_directories(amy INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/lib/amy
        ${CMAKE_CURRENT_LIST_DIR}/lib/amy/amy/src
)

target_link_libraries(amy INTERFACE
    pico_stdlib
    pico_multicore
    pico_rand
    hardware_dma
    hardware_pio
    hardware_irq
)
add_subdirectory(lib/pico-mpr121/lib mpr121)
add_subdirectory(lib/RP2040-Battery-Check battery_check)
add_subdirectory(lib/RP2040-Button button)

# Check if USE_MIDI is defined in config.h (not commented out)
file(READ "${CMAKE_CURRENT_LIST_DIR}/config.h" CONFIG_H_CONTENTS)
string(REPLACE "\n" ";" CONFIG_LINES "${CONFIG_H_CONTENTS}")
set(USE_MIDI_ENABLED FALSE)
foreach(LINE ${CONFIG_LINES})
    string(STRIP "${LINE}" STRIPPED_LINE)
    if(STRIPPED_LINE MATCHES "^#define USE_MIDI")
        set(USE_MIDI_ENABLED TRUE)
        break()
    endif()
endforeach()
if(USE_MIDI_ENABLED)
    message(STATUS "MIDI enabled - TinyUSB will be linked")
    message(STATUS "USB stdio disabled")
else()
    message(STATUS "MIDI disabled - TinyUSB will not be linked")
    message(STATUS "USB stdio enabled")
endif()

pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/audio/audio_i2s.pio)

target_sources(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/touch.c
        ${CMAKE_CURRENT_LIST_DIR}/state_data.c
        ${CMAKE_CURRENT_LIST_DIR}/flash.c
        ${CMAKE_CURRENT_LIST_DIR}/fretboard.c
        ${CMAKE_CURRENT_LIST_DIR}/directional_switch.c
	${CMAKE_CURRENT_LIST_DIR}/display/display.c
        ${CMAKE_CURRENT_LIST_DIR}/display/ui_items.c
        ${CMAKE_CURRENT_LIST_DIR}/global_filter.c
        ${CMAKE_CURRENT_LIST_DIR}/global_distortion.c
        ${CMAKE_CURRENT_LIST_DIR}/lib/pico-ssd1306/ssd1306.c
        ${CMAKE_CURRENT_LIST_DIR}/audio/audio_buffer.c
        ${CMAKE_CURRENT_LIST_DIR}/audio/audio_i2s.c
        )

if(USE_MIDI_ENABLED)
    target_sources(${PROJECT_NAME} PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/inc/tinyusb-midi/usb_descriptors.c
            )
    target_include_directories(${PROJECT_NAME} PUBLIC
            ${CMAKE_CURRENT_LIST_DIR}/inc/tinyusb-midi
            )
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/lib/pico-ssd1306
        ${CMAKE_CURRENT_BINARY_DIR}/generated
        )

target_link_libraries(${PROJECT_NAME} PRIVATE
        pico_stdlib
        amy
        hardware_i2c
        pico-mpr121
        battery_check
        pico_unique_id
        button
        )

if(USE_MIDI_ENABLED)
    target_link_libraries(${PROJECT_NAME} PRIVATE
            tinyusb_device
            tinyusb_board
            )
endif()

pico_add_extra_outputs(${PROJECT_NAME})

if(USE_MIDI_ENABLED)
    pico_enable_stdio_usb(${PROJECT_NAME} 0)
    pico_enable_stdio_uart(${PROJECT_NAME} 1)
else()
    pico_enable_stdio_usb(${PROJECT_NAME} 1)
    pico_enable_stdio_uart(${PROJECT_NAME} 0)
endif()