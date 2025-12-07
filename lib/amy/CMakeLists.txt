set(TARGET_NAME "amy")

if (NOT TARGET ${TARGET_NAME})
    add_library(${TARGET_NAME} INTERFACE)

    target_sources(${TARGET_NAME} INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/amy/src/algorithms.c
            ${CMAKE_CURRENT_LIST_DIR}/amy/src/amy.c
            ${CMAKE_CURRENT_LIST_DIR}/amy/src/amy_midi.c #new
            ${CMAKE_CURRENT_LIST_DIR}/amy/src/api.c #new
            ${CMAKE_CURRENT_LIST_DIR}/amy/src/custom.c
            ${CMAKE_CURRENT_LIST_DIR}/amy/src/delay.c
            ${CMAKE_CURRENT_LIST_DIR}/amy/src/envelope.c
            ${CMAKE_CURRENT_LIST_DIR}/amy/src/examples.c
            ${CMAKE_CURRENT_LIST_DIR}/amy/src/filters.c
            ${CMAKE_CURRENT_LIST_DIR}/amy/src/i2s.c #new
            ${CMAKE_CURRENT_LIST_DIR}/amy/src/instrument.c #new
            ${CMAKE_CURRENT_LIST_DIR}/amy/src/interp_partials.c
            ${CMAKE_CURRENT_LIST_DIR}/amy/src/libminiaudio-audio.c #new
            ${CMAKE_CURRENT_LIST_DIR}/amy/src/log2_exp2.c
            ${CMAKE_CURRENT_LIST_DIR}/amy/src/midi_mappings.c #new
            ${CMAKE_CURRENT_LIST_DIR}/amy/src/oscillators.c
            ${CMAKE_CURRENT_LIST_DIR}/amy/src/parse.c #new
            ${CMAKE_CURRENT_LIST_DIR}/amy/src/patches.c
            ${CMAKE_CURRENT_LIST_DIR}/amy/src/pcm.c
            ${CMAKE_CURRENT_LIST_DIR}/amy/src/pyamy.c #new
            ${CMAKE_CURRENT_LIST_DIR}/amy/src/sequencer.c
            ${CMAKE_CURRENT_LIST_DIR}/amy/src/transfer.c
            ${CMAKE_CURRENT_LIST_DIR}/amy/src/usb.c #new
    )
    
    target_include_directories(${TARGET_NAME} INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}
            ${CMAKE_CURRENT_LIST_DIR}/amy/src
    )

    target_link_libraries(${TARGET_NAME} INTERFACE
        pico_stdlib
        pico_audio_i2s
        pico_multicore
        pico_rand
        hardware_dma
        hardware_pio
        hardware_irq
    )
endif()
