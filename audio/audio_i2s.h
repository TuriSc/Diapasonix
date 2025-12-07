/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Adapted from pico-extras/src/rp2_common/pico_audio_i2s/include/pico/audio_i2s.h
 * Converted from C++ to C for this project.
 */

#ifndef AUDIO_I2S_H_
#define AUDIO_I2S_H_

#include "audio_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PICO_AUDIO_I2S_DMA_IRQ
#define PICO_AUDIO_I2S_DMA_IRQ 0
#endif

#ifndef PICO_AUDIO_I2S_PIO
#define PICO_AUDIO_I2S_PIO 0
#endif

#if !(PICO_AUDIO_I2S_DMA_IRQ == 0 || PICO_AUDIO_I2S_DMA_IRQ == 1)
#error PICO_AUDIO_I2S_DMA_IRQ must be 0 or 1
#endif

#if !(PICO_AUDIO_I2S_PIO == 0 || PICO_AUDIO_I2S_PIO == 1)
#error PICO_AUDIO_I2S_PIO must be 0 or 1
#endif

#ifndef PICO_AUDIO_I2S_MAX_CHANNELS
#define PICO_AUDIO_I2S_MAX_CHANNELS 2u
#endif

#ifndef PICO_AUDIO_I2S_BUFFERS_PER_CHANNEL
#define PICO_AUDIO_I2S_BUFFERS_PER_CHANNEL 3u
#endif

#ifndef PICO_AUDIO_I2S_BUFFER_SAMPLE_LENGTH
#define PICO_AUDIO_I2S_BUFFER_SAMPLE_LENGTH 576u
#endif

#ifndef PICO_AUDIO_I2S_SILENCE_BUFFER_SAMPLE_LENGTH
#define PICO_AUDIO_I2S_SILENCE_BUFFER_SAMPLE_LENGTH 256u
#endif

#ifndef PICO_AUDIO_I2S_NOOP
#define PICO_AUDIO_I2S_NOOP 0
#endif

#ifndef PICO_AUDIO_I2S_MONO_INPUT
#define PICO_AUDIO_I2S_MONO_INPUT 0
#endif

#ifndef PICO_AUDIO_I2S_MONO_OUTPUT
#define PICO_AUDIO_I2S_MONO_OUTPUT 0
#endif

#ifndef PICO_AUDIO_I2S_DATA_PIN
#define PICO_AUDIO_I2S_DATA_PIN 28
#endif

#ifndef PICO_AUDIO_I2S_CLOCK_PIN_BASE
#define PICO_AUDIO_I2S_CLOCK_PIN_BASE 26
#endif

#ifndef PICO_AUDIO_I2S_CLOCK_PINS_SWAPPED
#define PICO_AUDIO_I2S_CLOCK_PINS_SWAPPED 0
#endif

typedef struct audio_i2s_config {
    uint8_t data_pin;
    uint8_t clock_pin_base;
    uint8_t dma_channel;
    uint8_t pio_sm;
} audio_i2s_config_t;

const audio_format_t *audio_i2s_setup(const audio_format_t *intended_audio_format,
                                     const audio_i2s_config_t *config);

bool audio_i2s_connect_thru(audio_buffer_pool_t *producer, audio_connection_t *connection);

bool audio_i2s_connect(audio_buffer_pool_t *producer);

bool audio_i2s_connect_s8(audio_buffer_pool_t *producer);

bool audio_i2s_connect_extra(audio_buffer_pool_t *producer, bool buffer_on_give, uint buffer_count,
                             uint samples_per_buffer, audio_connection_t *connection);

void audio_i2s_set_enabled(bool enabled);

#ifdef __cplusplus
}
#endif

#endif //AUDIO_I2S_H_



