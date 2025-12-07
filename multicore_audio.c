#include "multicore_audio.h"
#include "global_filter.h"
#include "global_distortion.h"
#include <math.h>

extern struct audio_buffer_pool *ap;

int32_t await_message_from_other_core() {
    uint32_t timeout_start = time_us_32();
    const uint32_t TIMEOUT_US = 100000;  // 100ms timeout
    
    while (!multicore_fifo_rvalid()) {
        if (time_us_32() - timeout_start > TIMEOUT_US) {
            return -1;
        }
        sleep_us(10);
    }
    
    return multicore_fifo_pop_blocking();
}

void send_message_to_other_core(int32_t t) {
    multicore_fifo_push_blocking(t);
}

void rp2040_fill_audio_buffer() {
    amy_execute_deltas();
    
    // Send message to Core1 to start processing
    multicore_fifo_push_blocking(32);
    
    // Core0 renders first half of oscillators
    amy_render(0, AMY_OSCS/2, 0);
    
    // Wait for Core1 to finish second half with timeout
    uint32_t timeout_start = time_us_32();
    const uint32_t TIMEOUT_US = 5000;  // 5ms timeout
    
    while (!multicore_fifo_rvalid()) {
        if (time_us_32() - timeout_start > TIMEOUT_US) {
            break;
        }
        sleep_us(10);
    }
    
    if (multicore_fifo_rvalid()) {
        multicore_fifo_pop_blocking(); // Consume response
    }
    
    // Get the final combined buffer
    int16_t *block = amy_fill_buffer();
    
    // Apply global effects if enabled
    if (block != NULL) {
        global_distortion_process(block, AMY_BLOCK_SIZE);
        global_filter_process(block, AMY_BLOCK_SIZE);
    }
    
    struct audio_buffer *buffer = take_audio_buffer(ap, true);
    if (!buffer) {
        return;
    }
    
    int16_t *samples = (int16_t *) buffer->buffer->bytes;
    
    if (block != NULL) {
        // Copy AMY audio data to I2S buffer
        for (uint i = 0; i < AMY_BLOCK_SIZE * AMY_NCHANS; i++) {
            samples[i] = block[i];
        }
    } else {
        // Fill with silence if AMY returns NULL
        for (uint i = 0; i < AMY_BLOCK_SIZE * AMY_NCHANS; i++) {
            samples[i] = 0;
        }
    }
    
    buffer->sample_count = AMY_BLOCK_SIZE;
    give_audio_buffer(ap, buffer);
}

struct audio_buffer_pool *init_audio() {
    
    static audio_format_t audio_format = {
            .format = AUDIO_BUFFER_FORMAT_PCM_S16,
            .sample_freq = AMY_SAMPLE_RATE,
            .channel_count = AMY_NCHANS,
    };

    static struct audio_buffer_format producer_format = {
            .format = &audio_format,
            .sample_stride = sizeof(int16_t) * AMY_NCHANS,
    };

    struct audio_buffer_pool *producer_pool = audio_new_producer_pool(&producer_format, 3, AMY_BLOCK_SIZE);

    bool __unused ok;
    const struct audio_format *output_format;
    struct audio_i2s_config config = {
            .data_pin = PICO_AUDIO_I2S_DATA_PIN,
            .clock_pin_base = PICO_AUDIO_I2S_CLOCK_PIN_BASE,
            .dma_channel = 0,
            .pio_sm = 0,
    };

    output_format = audio_i2s_setup(&audio_format, &config);
    if (!output_format) {
        panic("PicoAudio: Unable to open audio device.\n");
    }

    ok = audio_i2s_connect(producer_pool);
    assert(ok);
    
    audio_i2s_set_enabled(true);

    return producer_pool;
}

// Delay while continuously filling audio buffers
void delay_ms(uint32_t ms) {
    uint32_t start = amy_sysclock();
    
    while(amy_sysclock() - start < ms) {
        rp2040_fill_audio_buffer();
    }
}

// Core1 audio processing function
void core1_main() {
    // Send ready signal to Core0
    multicore_fifo_push_blocking(99);
    
    uint32_t message_count = 0;
    
    while(1) {
        // Wait for message from Core0
        int32_t ret = multicore_fifo_pop_blocking();
        
        if (ret == 32) {
            static uint32_t render_count = 0;
            render_count++;
            
            
            // Add null pointer checks
            extern SAMPLE ** fbl;
            extern SAMPLE ** per_osc_fb;
            // Check algorithm scratch arrays
            extern SAMPLE ***scratch;
            
            // Validate memory pointers (only check once per session)
            static bool memory_checked = false;
            static bool memory_valid = false;
            
            if (!memory_checked) {
                // Check if pointers look valid (in reasonable memory range)
                uintptr_t addr_fbl1 = (uintptr_t)fbl[1];
                uintptr_t addr_per_osc_fb1 = (uintptr_t)per_osc_fb[1];
                uintptr_t addr_scratch1 = (uintptr_t)scratch[1];
                
                // Valid RP2350 RAM range is roughly 0x20000000 to 0x20080000
                bool fbl1_valid = (addr_fbl1 >= 0x20000000 && addr_fbl1 < 0x20080000);
                bool per_osc_fb1_valid = (addr_per_osc_fb1 >= 0x20000000 && addr_per_osc_fb1 < 0x20080000);
                bool scratch1_valid = (addr_scratch1 >= 0x20000000 && addr_scratch1 < 0x20080000);
                
                memory_valid = fbl1_valid && per_osc_fb1_valid && scratch1_valid;
                memory_checked = true;
                
                // printf("Core1: Memory validation - fbl[1]:%s per_osc_fb[1]:%s scratch[1]:%s\n",
                //        fbl1_valid ? "OK" : "BAD", 
                //        per_osc_fb1_valid ? "OK" : "BAD",
                //        scratch1_valid ? "OK" : "BAD");
            }
            
            // Check if memory is valid
            if (!memory_valid) {
                static bool corruption_logged = false;
                
                if (!corruption_logged) {
                    printf("Core1: ERROR - Invalid memory pointers detected! Core1 rendering disabled.\n");
                    printf("Core1: System will continue with Core0-only audio rendering.\n");
                    corruption_logged = true;
                }
                
                // Send response immediately to avoid Core0 timeout
                multicore_fifo_push_blocking(64);
                continue;
            }
            
            // Render second half of oscillators
            amy_render(AMY_OSCS/2, AMY_OSCS, 1);
            
            
            // Send completion signal back to Core0
            multicore_fifo_push_blocking(64);
        } else {
            printf("Core1: Unexpected message: %ld (expected 32)\n", ret);
        }
        
        message_count++;
    }
}
