/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Adapted from pico-extras/src/common/pico_audio/include/pico/audio.h
 * Converted from C++ to C for this project.
 */

#ifndef AUDIO_BUFFER_H_
#define AUDIO_BUFFER_H_

#include "pico.h"
#include "hardware/sync.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SPINLOCK_ID_AUDIO_FREE_LIST_LOCK
#define SPINLOCK_ID_AUDIO_FREE_LIST_LOCK 6
#endif

#ifndef SPINLOCK_ID_AUDIO_PREPARED_LISTS_LOCK
#define SPINLOCK_ID_AUDIO_PREPARED_LISTS_LOCK 7
#endif

#define AUDIO_BUFFER_FORMAT_PCM_S16 1
#define AUDIO_BUFFER_FORMAT_PCM_S8 2
#define AUDIO_BUFFER_FORMAT_PCM_U16 3
#define AUDIO_BUFFER_FORMAT_PCM_U8 4

typedef struct mem_buffer {
    size_t size;
    uint8_t *bytes;
    uint8_t flags;
} mem_buffer_t;

typedef struct audio_format {
    uint32_t sample_freq;
    uint16_t format;
    uint16_t channel_count;
} audio_format_t;

typedef struct audio_buffer_format {
    const audio_format_t *format;
    uint16_t sample_stride;
} audio_buffer_format_t;

typedef struct audio_buffer {
    mem_buffer_t *buffer;
    const audio_buffer_format_t *format;
    uint32_t sample_count;
    uint32_t max_sample_count;
    uint32_t user_data;
    struct audio_buffer *next;
} audio_buffer_t;

typedef struct audio_connection audio_connection_t;

typedef struct audio_buffer_pool {
    enum {
        ac_producer, ac_consumer
    } type;
    const audio_format_t *format;
    audio_connection_t *connection;
    spin_lock_t *free_list_spin_lock;
    audio_buffer_t *free_list;
    spin_lock_t *prepared_list_spin_lock;
    audio_buffer_t *prepared_list;
    audio_buffer_t *prepared_list_tail;
} audio_buffer_pool_t;

struct audio_connection {
    audio_buffer_t *(*producer_pool_take)(audio_connection_t *connection, bool block);
    void (*producer_pool_give)(audio_connection_t *connection, audio_buffer_t *buffer);
    audio_buffer_t *(*consumer_pool_take)(audio_connection_t *connection, bool block);
    void (*consumer_pool_give)(audio_connection_t *connection, audio_buffer_t *buffer);
    audio_buffer_pool_t *producer_pool;
    audio_buffer_pool_t *consumer_pool;
};

audio_buffer_pool_t *audio_new_producer_pool(audio_buffer_format_t *format, int buffer_count,
                                             int buffer_sample_count);

audio_buffer_pool_t *audio_new_consumer_pool(audio_buffer_format_t *format, int buffer_count,
                                             int buffer_sample_count);

audio_buffer_t *audio_new_wrapping_buffer(audio_buffer_format_t *format, mem_buffer_t *buffer);

audio_buffer_t *audio_new_buffer(audio_buffer_format_t *format, int buffer_sample_count);

void audio_init_buffer(audio_buffer_t *audio_buffer, audio_buffer_format_t *format, int buffer_sample_count);

void give_audio_buffer(audio_buffer_pool_t *ac, audio_buffer_t *buffer);

audio_buffer_t *take_audio_buffer(audio_buffer_pool_t *ac, bool block);

void audio_complete_connection(audio_connection_t *connection, audio_buffer_pool_t *producer,
                               audio_buffer_pool_t *consumer);

audio_buffer_t *get_free_audio_buffer(audio_buffer_pool_t *context, bool block);

void queue_free_audio_buffer(audio_buffer_pool_t *context, audio_buffer_t *ab);

audio_buffer_t *get_full_audio_buffer(audio_buffer_pool_t *context, bool block);

void queue_full_audio_buffer(audio_buffer_pool_t *context, audio_buffer_t *ab);

void consumer_pool_give_buffer_default(audio_connection_t *connection, audio_buffer_t *buffer);

audio_buffer_t *consumer_pool_take_buffer_default(audio_connection_t *connection, bool block);

void producer_pool_give_buffer_default(audio_connection_t *connection, audio_buffer_t *buffer);

audio_buffer_t *producer_pool_take_buffer_default(audio_connection_t *connection, bool block);

audio_buffer_t *mono_to_mono_consumer_take(audio_connection_t *connection, bool block);

audio_buffer_t *stereo_to_stereo_consumer_take(audio_connection_t *connection, bool block);

audio_buffer_t *mono_to_stereo_consumer_take(audio_connection_t *connection, bool block);

void stereo_to_stereo_producer_give(audio_connection_t *connection, audio_buffer_t *buffer);

struct buffer_copying_on_consumer_take_connection {
    struct audio_connection core;
    audio_buffer_t *current_producer_buffer;
    uint32_t current_producer_buffer_pos;
};

struct producer_pool_blocking_give_connection {
    audio_connection_t core;
    audio_buffer_t *current_consumer_buffer;
    uint32_t current_consumer_buffer_pos;
};

#ifdef __cplusplus
}
#endif

#endif //AUDIO_BUFFER_H_



