/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Adapted from pico-extras/src/common/pico_audio/audio.cpp
 * Converted from C++ to C for this project.
 */

#include "audio_buffer.h"
#include <string.h>
#include <assert.h>

static inline audio_buffer_t *list_remove_head(audio_buffer_t **phead) {
    audio_buffer_t *ab = *phead;
    if (ab) {
        *phead = ab->next;
        ab->next = NULL;
    }
    return ab;
}

static inline audio_buffer_t *list_remove_head_with_tail(audio_buffer_t **phead,
                                                          audio_buffer_t **ptail) {
    audio_buffer_t *ab = *phead;
    if (ab) {
        *phead = ab->next;
        if (!ab->next) {
            assert(*ptail == ab);
            *ptail = NULL;
        } else {
            ab->next = NULL;
        }
    }
    return ab;
}

static inline void list_prepend(audio_buffer_t **phead, audio_buffer_t *ab) {
    assert(ab->next == NULL);
    assert(ab != *phead);
    ab->next = *phead;
    *phead = ab;
}

static inline void list_append_with_tail(audio_buffer_t **phead, audio_buffer_t **ptail,
                                         audio_buffer_t *ab) {
    assert(ab->next == NULL);
    assert(ab != *phead);
    assert(ab != *ptail);
    if (!*phead) {
        assert(!*ptail);
        *ptail = ab;
        list_prepend(phead, ab);
    } else {
        (*ptail)->next = ab;
        *ptail = ab;
    }
}

audio_buffer_t *get_free_audio_buffer(audio_buffer_pool_t *context, bool block) {
    audio_buffer_t *ab;
    do {
        uint32_t save = spin_lock_blocking(context->free_list_spin_lock);
        ab = list_remove_head(&context->free_list);
        spin_unlock(context->free_list_spin_lock, save);
        if (ab || !block) break;
        __wfe();
    } while (true);
    return ab;
}

void queue_free_audio_buffer(audio_buffer_pool_t *context, audio_buffer_t *ab) {
    assert(!ab->next);
    uint32_t save = spin_lock_blocking(context->free_list_spin_lock);
    list_prepend(&context->free_list, ab);
    spin_unlock(context->free_list_spin_lock, save);
    __sev();
}

audio_buffer_t *get_full_audio_buffer(audio_buffer_pool_t *context, bool block) {
    audio_buffer_t *ab;
    do {
        uint32_t save = spin_lock_blocking(context->prepared_list_spin_lock);
        ab = list_remove_head_with_tail(&context->prepared_list, &context->prepared_list_tail);
        spin_unlock(context->prepared_list_spin_lock, save);
        if (ab || !block) break;
        __wfe();
    } while (true);
    return ab;
}

void queue_full_audio_buffer(audio_buffer_pool_t *context, audio_buffer_t *ab) {
    assert(!ab->next);
    uint32_t save = spin_lock_blocking(context->prepared_list_spin_lock);
    list_append_with_tail(&context->prepared_list, &context->prepared_list_tail, ab);
    spin_unlock(context->prepared_list_spin_lock, save);
    __sev();
}

void producer_pool_give_buffer_default(audio_connection_t *connection, audio_buffer_t *buffer) {
    queue_full_audio_buffer(connection->producer_pool, buffer);
}

audio_buffer_t *producer_pool_take_buffer_default(audio_connection_t *connection, bool block) {
    return get_free_audio_buffer(connection->producer_pool, block);
}

void consumer_pool_give_buffer_default(audio_connection_t *connection, audio_buffer_t *buffer) {
    queue_free_audio_buffer(connection->consumer_pool, buffer);
}

audio_buffer_t *consumer_pool_take_buffer_default(audio_connection_t *connection, bool block) {
    return get_full_audio_buffer(connection->consumer_pool, block);
}

static audio_connection_t connection_default = {
    .producer_pool_take = producer_pool_take_buffer_default,
    .producer_pool_give = producer_pool_give_buffer_default,
    .consumer_pool_take = consumer_pool_take_buffer_default,
    .consumer_pool_give = consumer_pool_give_buffer_default,
};

static mem_buffer_t *buffer_alloc(size_t size) {
    mem_buffer_t *b = (mem_buffer_t *)malloc(sizeof(mem_buffer_t));
    if (b) {
        b->bytes = (uint8_t *)calloc(1, size);
        if (b->bytes) {
            b->size = size;
            b->flags = 0;
        } else {
            free(b);
            b = NULL;
        }
    }
    return b;
}

audio_buffer_t *audio_new_buffer(audio_buffer_format_t *format, int buffer_sample_count) {
    audio_buffer_t *buffer = (audio_buffer_t *)calloc(1, sizeof(audio_buffer_t));
    if (buffer) {
        audio_init_buffer(buffer, format, buffer_sample_count);
    }
    return buffer;
}

void audio_init_buffer(audio_buffer_t *audio_buffer, audio_buffer_format_t *format, int buffer_sample_count) {
    audio_buffer->format = format;
    audio_buffer->buffer = buffer_alloc(buffer_sample_count * format->sample_stride);
    audio_buffer->max_sample_count = buffer_sample_count;
    audio_buffer->sample_count = 0;
}

static audio_buffer_pool_t *
audio_new_buffer_pool(audio_buffer_format_t *format, int buffer_count, int buffer_sample_count) {
    audio_buffer_pool_t *ac = (audio_buffer_pool_t *)calloc(1, sizeof(audio_buffer_pool_t));
    if (!ac) return NULL;
    
    audio_buffer_t *audio_buffers = buffer_count ? (audio_buffer_t *)calloc(buffer_count, sizeof(audio_buffer_t)) : NULL;
    if (buffer_count && !audio_buffers) {
        free(ac);
        return NULL;
    }
    
    ac->format = format->format;
    for (int i = 0; i < buffer_count; i++) {
        audio_init_buffer(audio_buffers + i, format, buffer_sample_count);
        audio_buffers[i].next = (i != buffer_count - 1) ? &audio_buffers[i + 1] : NULL;
    }
    ac->free_list_spin_lock = spin_lock_init(SPINLOCK_ID_AUDIO_FREE_LIST_LOCK);
    ac->free_list = audio_buffers;
    ac->prepared_list_spin_lock = spin_lock_init(SPINLOCK_ID_AUDIO_PREPARED_LISTS_LOCK);
    ac->prepared_list = NULL;
    ac->prepared_list_tail = NULL;
    ac->connection = &connection_default;
    return ac;
}

audio_buffer_pool_t *
audio_new_producer_pool(audio_buffer_format_t *format, int buffer_count, int buffer_sample_count) {
    audio_buffer_pool_t *ac = audio_new_buffer_pool(format, buffer_count, buffer_sample_count);
    if (ac) {
        ac->type = ac_producer;
    }
    return ac;
}

audio_buffer_pool_t *
audio_new_consumer_pool(audio_buffer_format_t *format, int buffer_count, int buffer_sample_count) {
    audio_buffer_pool_t *ac = audio_new_buffer_pool(format, buffer_count, buffer_sample_count);
    if (ac) {
        ac->type = ac_consumer;
    }
    return ac;
}

audio_buffer_t *audio_new_wrapping_buffer(audio_buffer_format_t *format, mem_buffer_t *buffer) {
    audio_buffer_t *audio_buffer = (audio_buffer_t *)calloc(1, sizeof(audio_buffer_t));
    if (audio_buffer) {
        audio_buffer->format = format;
        audio_buffer->buffer = buffer;
        audio_buffer->max_sample_count = buffer->size / format->sample_stride;
        audio_buffer->sample_count = 0;
        audio_buffer->next = NULL;
    }
    return audio_buffer;
}

void audio_complete_connection(audio_connection_t *connection, audio_buffer_pool_t *producer_pool,
                               audio_buffer_pool_t *consumer_pool) {
    assert(producer_pool->type == ac_producer);
    assert(consumer_pool->type == ac_consumer);
    producer_pool->connection = connection;
    consumer_pool->connection = connection;
    connection->producer_pool = producer_pool;
    connection->consumer_pool = consumer_pool;
}

void give_audio_buffer(audio_buffer_pool_t *ac, audio_buffer_t *buffer) {
    buffer->user_data = 0;
    assert(ac->connection);
    if (ac->type == ac_producer)
        ac->connection->producer_pool_give(ac->connection, buffer);
    else
        ac->connection->consumer_pool_give(ac->connection, buffer);
}

audio_buffer_t *take_audio_buffer(audio_buffer_pool_t *ac, bool block) {
    assert(ac->connection);
    if (ac->type == ac_producer)
        return ac->connection->producer_pool_take(ac->connection, block);
    else
        return ac->connection->consumer_pool_take(ac->connection, block);
}

static void stereo_to_stereo_copy(int16_t *dest, const int16_t *src, uint sample_count) {
    memcpy(dest, src, sample_count * 4);
}

static void mono_to_mono_copy(int16_t *dest, const int16_t *src, uint sample_count) {
    memcpy(dest, src, sample_count * 2);
}

static void mono_to_stereo_copy(int16_t *dest, const int16_t *src, uint sample_count) {
    for (uint i = 0; i < sample_count; i++) {
        int16_t mono_sample = src[i];
        dest[i * 2] = mono_sample;
        dest[i * 2 + 1] = mono_sample;
    }
}

static audio_buffer_t *consumer_take_copy(audio_connection_t *connection, bool block,
                                           void (*copy_fn)(int16_t *, const int16_t *, uint),
                                           uint dest_channels, uint src_channels) {
    struct buffer_copying_on_consumer_take_connection *cc = 
        (struct buffer_copying_on_consumer_take_connection *)connection;
    
    audio_buffer_t *buffer = get_free_audio_buffer(cc->core.consumer_pool, block);
    if (!buffer) return NULL;
    
    assert(buffer->format->sample_stride == dest_channels * 2);
    
    uint32_t pos = 0;
    while (pos < buffer->max_sample_count) {
        if (!cc->current_producer_buffer) {
            cc->current_producer_buffer = get_full_audio_buffer(cc->core.producer_pool, block);
            if (!cc->current_producer_buffer) {
                assert(!block);
                if (!pos) {
                    queue_free_audio_buffer(cc->core.consumer_pool, buffer);
                    return NULL;
                }
                break;
            }
            assert(cc->current_producer_buffer->format->format->channel_count == src_channels);
            assert(cc->current_producer_buffer->format->sample_stride == src_channels * 2);
            cc->current_producer_buffer_pos = 0;
        }
        
        uint sample_count = buffer->max_sample_count - pos;
        uint remaining = cc->current_producer_buffer->sample_count - cc->current_producer_buffer_pos;
        if (sample_count > remaining) {
            sample_count = remaining;
        }
        
        copy_fn(
            ((int16_t *)buffer->buffer->bytes) + pos * dest_channels,
            ((int16_t *)cc->current_producer_buffer->buffer->bytes) + 
            cc->current_producer_buffer_pos * src_channels,
            sample_count);
        
        pos += sample_count;
        cc->current_producer_buffer_pos += sample_count;
        
        if (cc->current_producer_buffer_pos == cc->current_producer_buffer->sample_count) {
            queue_free_audio_buffer(cc->core.producer_pool, cc->current_producer_buffer);
            cc->current_producer_buffer = NULL;
        }
    }
    buffer->sample_count = pos;
    return buffer;
}

audio_buffer_t *mono_to_mono_consumer_take(audio_connection_t *connection, bool block) {
    return consumer_take_copy(connection, block, mono_to_mono_copy, 1, 1);
}

audio_buffer_t *stereo_to_stereo_consumer_take(audio_connection_t *connection, bool block) {
    return consumer_take_copy(connection, block, stereo_to_stereo_copy, 2, 2);
}

audio_buffer_t *mono_to_stereo_consumer_take(audio_connection_t *connection, bool block) {
    return consumer_take_copy(connection, block, mono_to_stereo_copy, 2, 1);
}

static void producer_give_copy(audio_connection_t *connection, audio_buffer_t *buffer,
                              void (*copy_fn)(int16_t *, const int16_t *, uint),
                              uint dest_channels, uint src_channels) {
    struct producer_pool_blocking_give_connection *pbc = 
        (struct producer_pool_blocking_give_connection *)connection;
    
    uint32_t pos = 0;
    while (pos < buffer->sample_count) {
        if (!pbc->current_consumer_buffer) {
            pbc->current_consumer_buffer = get_free_audio_buffer(pbc->core.consumer_pool, true);
            pbc->current_consumer_buffer_pos = 0;
        }
        
        uint sample_count = buffer->sample_count - pos;
        uint remaining = pbc->current_consumer_buffer->max_sample_count - pbc->current_consumer_buffer_pos;
        if (sample_count > remaining) {
            sample_count = remaining;
        }
        
        assert(buffer->format->sample_stride == src_channels * 2);
        assert(buffer->format->format->channel_count == src_channels);
        
        copy_fn(
            ((int16_t *)pbc->current_consumer_buffer->buffer->bytes) + 
            pbc->current_consumer_buffer_pos * dest_channels,
            ((int16_t *)buffer->buffer->bytes) + pos * src_channels,
            sample_count);
        
        pos += sample_count;
        pbc->current_consumer_buffer_pos += sample_count;
        
        if (pbc->current_consumer_buffer_pos == pbc->current_consumer_buffer->max_sample_count) {
            pbc->current_consumer_buffer->sample_count = pbc->current_consumer_buffer->max_sample_count;
            queue_full_audio_buffer(pbc->core.consumer_pool, pbc->current_consumer_buffer);
            pbc->current_consumer_buffer = NULL;
        }
    }
    
    assert(pos == buffer->sample_count);
    queue_free_audio_buffer(pbc->core.producer_pool, buffer);
}

void stereo_to_stereo_producer_give(audio_connection_t *connection, audio_buffer_t *buffer) {
    producer_give_copy(connection, buffer, stereo_to_stereo_copy, 2, 2);
}

