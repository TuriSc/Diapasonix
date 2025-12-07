#ifndef MULTICORE_AUDIO_H_
#define MULTICORE_AUDIO_H_

#include <stdio.h>
#include <math.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "amy.h"

#include "config.h"
#include "audio/audio_i2s.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t await_message_from_other_core();
void send_message_to_other_core(int32_t t);
void fill_audio_buffer();
struct audio_buffer_pool *init_audio();
void delay_ms(uint32_t ms);
void core1_main();

#ifdef __cplusplus
}
#endif

#endif
