#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/multicore.h"
#include "config.h"
#include "state_data.h"
#include "display/display.h"
#include "multicore_audio.h"
#include "audio/audio_i2s.h"
#include "amy.h"
#include <string.h>

extern ssd1306_t display;

static alarm_id_t flash_write_alarm_id;
static bool flash_write_pending = false;  // Flag to indicate flash write is needed
static bool flash_write_in_progress = false;  // Flag to prevent re-entrancy and audio operations during write
static uint8_t pending_preset_override = 0xFF;  // 0xFF = use current preset from flash, otherwise use this preset number

// Helper function to copy float to uint8_t buffer (4 bytes)
static inline void pack_float(uint8_t *buffer, float value) {
    memcpy(buffer, &value, sizeof(float));
}

// Helper function to unpack float from uint8_t buffer (4 bytes)
static inline float unpack_float(const uint8_t *buffer) {
    float value;
    memcpy(&value, buffer, sizeof(value));
    return value;
}

// Helper function to copy int16_t to uint8_t buffer (2 bytes)
static inline void pack_int16(uint8_t *buffer, int16_t value) {
    buffer[0] = (uint8_t)(value & 0xFF);
    buffer[1] = (uint8_t)((value >> 8) & 0xFF);
}

// Helper function to unpack int16_t from uint8_t buffer (2 bytes)
static inline int16_t unpack_int16(const uint8_t *buffer) {
    return (int16_t)((buffer[1] << 8) | buffer[0]);
}

// Helper function to copy int32_t to uint8_t buffer (4 bytes)
static inline void pack_int32(uint8_t *buffer, int32_t value) {
    buffer[0] = (uint8_t)(value & 0xFF);
    buffer[1] = (uint8_t)((value >> 8) & 0xFF);
    buffer[2] = (uint8_t)((value >> 16) & 0xFF);
    buffer[3] = (uint8_t)((value >> 24) & 0xFF);
}

// Helper function to unpack int32_t from uint8_t buffer (4 bytes)
static inline int32_t unpack_int32(const uint8_t *buffer) {
    return (int32_t)((buffer[3] << 24) | (buffer[2] << 16) | (buffer[1] << 8) | buffer[0]);
}

// Size of a single preset in bytes
// Structure:
//    1 (patch)
// +  5 (effects)
// + 12 (reverb)
// + 12 (chorus)
// + 12 (echo)
// +  8 (filter)
// +  8 (distortion)
// +  4 (strings)
// +  2 (capo)
// +  1 (playing_mode) = 65 bytes

#define PRESET_SIZE 65

// Offset calculations for preset storage
#define OFFSET_MAGIC 0
#define OFFSET_VERSION (OFFSET_MAGIC + MAGIC_NUMBER_LENGTH)
#define OFFSET_CURRENT_PRESET (OFFSET_VERSION + 1)
#define OFFSET_PRESET_0 (OFFSET_CURRENT_PRESET + 1)
#define OFFSET_PRESET_1 (OFFSET_PRESET_0 + PRESET_SIZE)
#define OFFSET_PRESET_2 (OFFSET_PRESET_1 + PRESET_SIZE)
#define OFFSET_PRESET_3 (OFFSET_PRESET_2 + PRESET_SIZE)
#define OFFSET_VOLUME (OFFSET_PRESET_3 + PRESET_SIZE)      // Volume (0-8)
#define OFFSET_CONTRAST (OFFSET_VOLUME + 1)                // Contrast (0-3)
#define OFFSET_LEFTHANDED (OFFSET_CONTRAST + 1)            // Lefthanded (0-1)
#define OFFSET_TIMING_SNAPSHOT_WINDOW (OFFSET_LEFTHANDED + 1)  // Timing snapshot window (4 bytes)
#define OFFSET_TIMING_STALE_TIMEOUT (OFFSET_TIMING_SNAPSHOT_WINDOW + 4)  // Timing stale timeout (4 bytes)
#define OFFSET_TIMING_VERY_RECENT (OFFSET_TIMING_STALE_TIMEOUT + 4)  // Timing very recent threshold (4 bytes)
#define OFFSET_TIMING_POST_STRUM (OFFSET_TIMING_VERY_RECENT + 4)  // Timing post strum threshold (4 bytes)
#define OFFSET_TIMING_RELEASE_DELAY (OFFSET_TIMING_POST_STRUM + 4)  // Timing release delay (4 bytes)

// Helper function to pack current state into a preset buffer
static void pack_preset(uint8_t *buffer, uint16_t *offset) {
    buffer[*offset + 0] = get_patch();
    *offset += 1;
    
    // Save effect states
    buffer[*offset + 0] = get_fx(REVERB) ? 1 : 0;
    buffer[*offset + 1] = get_fx(FILTER) ? 1 : 0;
    buffer[*offset + 2] = get_fx(CHORUS) ? 1 : 0;
    buffer[*offset + 3] = get_fx(ECHO) ? 1 : 0;
    buffer[*offset + 4] = get_fx(DISTORTION) ? 1 : 0;
    *offset += 5;
    
    // Save effect parameters (floats are 4 bytes each)
    pack_float(&buffer[*offset], get_reverb_liveness());
    *offset += 4;
    pack_float(&buffer[*offset], get_reverb_damping());
    *offset += 4;
    pack_float(&buffer[*offset], get_reverb_xover_hz());
    *offset += 4;
    
    pack_int32(&buffer[*offset], get_chorus_max_delay());
    *offset += 4;
    pack_float(&buffer[*offset], get_chorus_lfo_freq());
    *offset += 4;
    pack_float(&buffer[*offset], get_chorus_depth());
    *offset += 4;
    
    pack_float(&buffer[*offset], get_echo_delay_ms());
    *offset += 4;
    pack_float(&buffer[*offset], get_echo_feedback());
    *offset += 4;
    pack_float(&buffer[*offset], get_echo_filter_coef());
    *offset += 4;
    
    pack_float(&buffer[*offset], get_filter_freq_hz());
    *offset += 4;
    pack_float(&buffer[*offset], get_filter_resonance());
    *offset += 4;
    
    pack_float(&buffer[*offset], get_distortion_level());
    *offset += 4;
    pack_float(&buffer[*offset], get_distortion_gain());
    *offset += 4;
    
    // Save string pitches
    buffer[*offset + 0] = get_string_pitch(0);
    buffer[*offset + 1] = get_string_pitch(1);
    buffer[*offset + 2] = get_string_pitch(2);
    buffer[*offset + 3] = get_string_pitch(3);
    *offset += 4;
    
    // Save capo (2 bytes, int16_t)
    pack_int16(&buffer[*offset], get_capo());
    *offset += 2;
    
    // Save playing_mode flag
    buffer[*offset + 0] = get_playing_mode() ? 1 : 0;
    *offset += 1;
}

// Helper function to unpack a preset buffer into current state
static void unpack_preset(const uint8_t *buffer, uint16_t *offset) {
    // Load patch
    set_patch(buffer[*offset + 0]);
    *offset += 1;
    
    // Load effect states
    set_fx(REVERB, buffer[*offset + 0] != 0);
    set_fx(FILTER, buffer[*offset + 1] != 0);
    set_fx(CHORUS, buffer[*offset + 2] != 0);
    set_fx(ECHO, buffer[*offset + 3] != 0);
    set_fx(DISTORTION, buffer[*offset + 4] != 0);
    *offset += 5;
    
    // Load effect parameters
    set_reverb_liveness(unpack_float(&buffer[*offset]));
    *offset += 4;
    set_reverb_damping(unpack_float(&buffer[*offset]));
    *offset += 4;
    set_reverb_xover_hz(unpack_float(&buffer[*offset]));
    *offset += 4;
    
    set_chorus_max_delay(unpack_int32(&buffer[*offset]));
    *offset += 4;
    set_chorus_lfo_freq(unpack_float(&buffer[*offset]));
    *offset += 4;
    set_chorus_depth(unpack_float(&buffer[*offset]));
    *offset += 4;
    
    set_echo_delay_ms(unpack_float(&buffer[*offset]));
    *offset += 4;
    set_echo_feedback(unpack_float(&buffer[*offset]));
    *offset += 4;
    set_echo_filter_coef(unpack_float(&buffer[*offset]));
    *offset += 4;
    
    set_filter_freq_hz(unpack_float(&buffer[*offset]));
    *offset += 4;
    set_filter_resonance(unpack_float(&buffer[*offset]));
    *offset += 4;
    
    set_distortion_level(unpack_float(&buffer[*offset]));
    *offset += 4;
    set_distortion_gain(unpack_float(&buffer[*offset]));
    *offset += 4;
    
    // Load string pitches
    set_string_pitch(0, buffer[*offset + 0]);
    set_string_pitch(1, buffer[*offset + 1]);
    set_string_pitch(2, buffer[*offset + 2]);
    set_string_pitch(3, buffer[*offset + 3]);
    *offset += 4;
    
    // Load capo
    set_capo(unpack_int16(&buffer[*offset]));
    *offset += 2;
    
    // Load playing_mode flag
    set_playing_mode(buffer[*offset + 0] != 0);
    *offset += 1;
}

// Helper function to load default preset values into current state
static void load_default_preset_to_state(uint8_t preset_num) {
    switch (preset_num) {
        case 0:
            set_patch(PRESET_0_PATCH);
            set_fx(REVERB, PRESET_0_FX_REVERB != 0);
            set_fx(FILTER, PRESET_0_FX_FILTER != 0);
            set_fx(CHORUS, PRESET_0_FX_CHORUS != 0);
            set_fx(ECHO, PRESET_0_FX_ECHO != 0);
            set_fx(DISTORTION, PRESET_0_FX_DISTORTION != 0);
            set_reverb_liveness(PRESET_0_REVERB_LIVENESS);
            set_reverb_damping(PRESET_0_REVERB_DAMPING);
            set_reverb_xover_hz(PRESET_0_REVERB_XOVER_HZ);
            set_chorus_max_delay(PRESET_0_CHORUS_MAX_DELAY);
            set_chorus_lfo_freq(PRESET_0_CHORUS_LFO_FREQ);
            set_chorus_depth(PRESET_0_CHORUS_DEPTH);
            set_echo_delay_ms(PRESET_0_ECHO_DELAY_MS);
            set_echo_feedback(PRESET_0_ECHO_FEEDBACK);
            set_echo_filter_coef(PRESET_0_ECHO_FILTER_COEF);
            set_filter_freq_hz(PRESET_0_FILTER_FREQ_HZ);
            set_filter_resonance(PRESET_0_FILTER_RESONANCE);
            set_distortion_level(PRESET_0_DISTORTION_LEVEL);
            set_distortion_gain(PRESET_0_DISTORTION_GAIN);
            set_string_pitch(0, PRESET_0_STRING_PITCH_0);
            set_string_pitch(1, PRESET_0_STRING_PITCH_1);
            set_string_pitch(2, PRESET_0_STRING_PITCH_2);
            set_string_pitch(3, PRESET_0_STRING_PITCH_3);
            set_capo(PRESET_0_CAPO);
            set_playing_mode(PRESET_0_PLAYING_MODE != 0);
            break;
        case 1:
            set_patch(PRESET_1_PATCH);
            set_fx(REVERB, PRESET_1_FX_REVERB != 0);
            set_fx(FILTER, PRESET_1_FX_FILTER != 0);
            set_fx(CHORUS, PRESET_1_FX_CHORUS != 0);
            set_fx(ECHO, PRESET_1_FX_ECHO != 0);
            set_fx(DISTORTION, PRESET_1_FX_DISTORTION != 0);
            set_reverb_liveness(PRESET_1_REVERB_LIVENESS);
            set_reverb_damping(PRESET_1_REVERB_DAMPING);
            set_reverb_xover_hz(PRESET_1_REVERB_XOVER_HZ);
            set_chorus_max_delay(PRESET_1_CHORUS_MAX_DELAY);
            set_chorus_lfo_freq(PRESET_1_CHORUS_LFO_FREQ);
            set_chorus_depth(PRESET_1_CHORUS_DEPTH);
            set_echo_delay_ms(PRESET_1_ECHO_DELAY_MS);
            set_echo_feedback(PRESET_1_ECHO_FEEDBACK);
            set_echo_filter_coef(PRESET_1_ECHO_FILTER_COEF);
            set_filter_freq_hz(PRESET_1_FILTER_FREQ_HZ);
            set_filter_resonance(PRESET_1_FILTER_RESONANCE);
            set_distortion_level(PRESET_1_DISTORTION_LEVEL);
            set_distortion_gain(PRESET_1_DISTORTION_GAIN);
            set_string_pitch(0, PRESET_1_STRING_PITCH_0);
            set_string_pitch(1, PRESET_1_STRING_PITCH_1);
            set_string_pitch(2, PRESET_1_STRING_PITCH_2);
            set_string_pitch(3, PRESET_1_STRING_PITCH_3);
            set_capo(PRESET_1_CAPO);
            set_playing_mode(PRESET_1_PLAYING_MODE != 0);
            break;
        case 2:
            set_patch(PRESET_2_PATCH);
            set_fx(REVERB, PRESET_2_FX_REVERB != 0);
            set_fx(FILTER, PRESET_2_FX_FILTER != 0);
            set_fx(CHORUS, PRESET_2_FX_CHORUS != 0);
            set_fx(ECHO, PRESET_2_FX_ECHO != 0);
            set_fx(DISTORTION, PRESET_2_FX_DISTORTION != 0);
            set_reverb_liveness(PRESET_2_REVERB_LIVENESS);
            set_reverb_damping(PRESET_2_REVERB_DAMPING);
            set_reverb_xover_hz(PRESET_2_REVERB_XOVER_HZ);
            set_chorus_max_delay(PRESET_2_CHORUS_MAX_DELAY);
            set_chorus_lfo_freq(PRESET_2_CHORUS_LFO_FREQ);
            set_chorus_depth(PRESET_2_CHORUS_DEPTH);
            set_echo_delay_ms(PRESET_2_ECHO_DELAY_MS);
            set_echo_feedback(PRESET_2_ECHO_FEEDBACK);
            set_echo_filter_coef(PRESET_2_ECHO_FILTER_COEF);
            set_filter_freq_hz(PRESET_2_FILTER_FREQ_HZ);
            set_filter_resonance(PRESET_2_FILTER_RESONANCE);
            set_distortion_level(PRESET_2_DISTORTION_LEVEL);
            set_distortion_gain(PRESET_2_DISTORTION_GAIN);
            set_string_pitch(0, PRESET_2_STRING_PITCH_0);
            set_string_pitch(1, PRESET_2_STRING_PITCH_1);
            set_string_pitch(2, PRESET_2_STRING_PITCH_2);
            set_string_pitch(3, PRESET_2_STRING_PITCH_3);
            set_capo(PRESET_2_CAPO);
            set_playing_mode(PRESET_2_PLAYING_MODE != 0);
            break;
        case 3:
            set_patch(PRESET_3_PATCH);
            set_fx(REVERB, PRESET_3_FX_REVERB != 0);
            set_fx(FILTER, PRESET_3_FX_FILTER != 0);
            set_fx(CHORUS, PRESET_3_FX_CHORUS != 0);
            set_fx(ECHO, PRESET_3_FX_ECHO != 0);
            set_fx(DISTORTION, PRESET_3_FX_DISTORTION != 0);
            set_reverb_liveness(PRESET_3_REVERB_LIVENESS);
            set_reverb_damping(PRESET_3_REVERB_DAMPING);
            set_reverb_xover_hz(PRESET_3_REVERB_XOVER_HZ);
            set_chorus_max_delay(PRESET_3_CHORUS_MAX_DELAY);
            set_chorus_lfo_freq(PRESET_3_CHORUS_LFO_FREQ);
            set_chorus_depth(PRESET_3_CHORUS_DEPTH);
            set_echo_delay_ms(PRESET_3_ECHO_DELAY_MS);
            set_echo_feedback(PRESET_3_ECHO_FEEDBACK);
            set_echo_filter_coef(PRESET_3_ECHO_FILTER_COEF);
            set_filter_freq_hz(PRESET_3_FILTER_FREQ_HZ);
            set_filter_resonance(PRESET_3_FILTER_RESONANCE);
            set_distortion_level(PRESET_3_DISTORTION_LEVEL);
            set_distortion_gain(PRESET_3_DISTORTION_GAIN);
            set_string_pitch(0, PRESET_3_STRING_PITCH_0);
            set_string_pitch(1, PRESET_3_STRING_PITCH_1);
            set_string_pitch(2, PRESET_3_STRING_PITCH_2);
            set_string_pitch(3, PRESET_3_STRING_PITCH_3);
            set_capo(PRESET_3_CAPO);
            set_playing_mode(PRESET_3_PLAYING_MODE != 0);
            break;
    }
    
    // Update audio engine with loaded values
    extern void update_patch(void);
    extern void update_fx(amy_fx_t fx);
    extern void update_tuning(void);
    extern void update_volume(void);
    update_patch();
    update_fx(REVERB);
    update_fx(CHORUS);
    update_fx(ECHO);
    update_fx(FILTER);
    update_fx(DISTORTION);
    update_tuning();
    update_volume();
}

// Helper function to pack default preset values into a buffer
static void pack_default_preset(uint8_t *buffer, uint16_t *offset, uint8_t preset_num) {
    // Save basic state
    switch (preset_num) {
        case 0:
            buffer[*offset + 0] = PRESET_0_PATCH;
            *offset += 1;
            buffer[*offset + 0] = PRESET_0_FX_REVERB;
            buffer[*offset + 1] = PRESET_0_FX_FILTER;
            buffer[*offset + 2] = PRESET_0_FX_CHORUS;
            buffer[*offset + 3] = PRESET_0_FX_ECHO;
            buffer[*offset + 4] = PRESET_0_FX_DISTORTION;
            *offset += 5;
            pack_float(&buffer[*offset], PRESET_0_REVERB_LIVENESS);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_0_REVERB_DAMPING);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_0_REVERB_XOVER_HZ);
            *offset += 4;
            pack_int32(&buffer[*offset], PRESET_0_CHORUS_MAX_DELAY);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_0_CHORUS_LFO_FREQ);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_0_CHORUS_DEPTH);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_0_ECHO_DELAY_MS);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_0_ECHO_FEEDBACK);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_0_ECHO_FILTER_COEF);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_0_FILTER_FREQ_HZ);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_0_FILTER_RESONANCE);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_0_DISTORTION_LEVEL);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_0_DISTORTION_GAIN);
            *offset += 4;
            buffer[*offset + 0] = PRESET_0_STRING_PITCH_0;
            buffer[*offset + 1] = PRESET_0_STRING_PITCH_1;
            buffer[*offset + 2] = PRESET_0_STRING_PITCH_2;
            buffer[*offset + 3] = PRESET_0_STRING_PITCH_3;
            *offset += 4;
            pack_int16(&buffer[*offset], PRESET_0_CAPO);
            *offset += 2;
            buffer[*offset + 0] = PRESET_0_PLAYING_MODE;
            *offset += 1;
            break;
        case 1:
            buffer[*offset + 0] = PRESET_1_PATCH;
            *offset += 1;
            buffer[*offset + 0] = PRESET_1_FX_REVERB;
            buffer[*offset + 1] = PRESET_1_FX_FILTER;
            buffer[*offset + 2] = PRESET_1_FX_CHORUS;
            buffer[*offset + 3] = PRESET_1_FX_ECHO;
            buffer[*offset + 4] = PRESET_1_FX_DISTORTION;
            *offset += 5;
            pack_float(&buffer[*offset], PRESET_1_REVERB_LIVENESS);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_1_REVERB_DAMPING);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_1_REVERB_XOVER_HZ);
            *offset += 4;
            pack_int32(&buffer[*offset], PRESET_1_CHORUS_MAX_DELAY);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_1_CHORUS_LFO_FREQ);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_1_CHORUS_DEPTH);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_1_ECHO_DELAY_MS);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_1_ECHO_FEEDBACK);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_1_ECHO_FILTER_COEF);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_1_FILTER_FREQ_HZ);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_1_FILTER_RESONANCE);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_1_DISTORTION_LEVEL);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_1_DISTORTION_GAIN);
            *offset += 4;
            buffer[*offset + 0] = PRESET_1_STRING_PITCH_0;
            buffer[*offset + 1] = PRESET_1_STRING_PITCH_1;
            buffer[*offset + 2] = PRESET_1_STRING_PITCH_2;
            buffer[*offset + 3] = PRESET_1_STRING_PITCH_3;
            *offset += 4;
            pack_int16(&buffer[*offset], PRESET_1_CAPO);
            *offset += 2;
            buffer[*offset + 0] = PRESET_1_PLAYING_MODE;
            *offset += 1;
            break;
        case 2:
            buffer[*offset + 0] = PRESET_2_PATCH;
            *offset += 1;
            buffer[*offset + 0] = PRESET_2_FX_REVERB;
            buffer[*offset + 1] = PRESET_2_FX_FILTER;
            buffer[*offset + 2] = PRESET_2_FX_CHORUS;
            buffer[*offset + 3] = PRESET_2_FX_ECHO;
            buffer[*offset + 4] = PRESET_2_FX_DISTORTION;
            *offset += 5;
            pack_float(&buffer[*offset], PRESET_2_REVERB_LIVENESS);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_2_REVERB_DAMPING);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_2_REVERB_XOVER_HZ);
            *offset += 4;
            pack_int32(&buffer[*offset], PRESET_2_CHORUS_MAX_DELAY);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_2_CHORUS_LFO_FREQ);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_2_CHORUS_DEPTH);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_2_ECHO_DELAY_MS);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_2_ECHO_FEEDBACK);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_2_ECHO_FILTER_COEF);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_2_FILTER_FREQ_HZ);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_2_FILTER_RESONANCE);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_2_DISTORTION_LEVEL);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_2_DISTORTION_GAIN);
            *offset += 4;
            buffer[*offset + 0] = PRESET_2_STRING_PITCH_0;
            buffer[*offset + 1] = PRESET_2_STRING_PITCH_1;
            buffer[*offset + 2] = PRESET_2_STRING_PITCH_2;
            buffer[*offset + 3] = PRESET_2_STRING_PITCH_3;
            *offset += 4;
            pack_int16(&buffer[*offset], PRESET_2_CAPO);
            *offset += 2;
            buffer[*offset + 0] = PRESET_2_PLAYING_MODE;
            *offset += 1;
            break;
        case 3:
            buffer[*offset + 0] = PRESET_3_PATCH;
            *offset += 1;
            buffer[*offset + 0] = PRESET_3_FX_REVERB;
            buffer[*offset + 1] = PRESET_3_FX_FILTER;
            buffer[*offset + 2] = PRESET_3_FX_CHORUS;
            buffer[*offset + 3] = PRESET_3_FX_ECHO;
            buffer[*offset + 4] = PRESET_3_FX_DISTORTION;
            *offset += 5;
            pack_float(&buffer[*offset], PRESET_3_REVERB_LIVENESS);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_3_REVERB_DAMPING);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_3_REVERB_XOVER_HZ);
            *offset += 4;
            pack_int32(&buffer[*offset], PRESET_3_CHORUS_MAX_DELAY);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_3_CHORUS_LFO_FREQ);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_3_CHORUS_DEPTH);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_3_ECHO_DELAY_MS);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_3_ECHO_FEEDBACK);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_3_ECHO_FILTER_COEF);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_3_FILTER_FREQ_HZ);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_3_FILTER_RESONANCE);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_3_DISTORTION_LEVEL);
            *offset += 4;
            pack_float(&buffer[*offset], PRESET_3_DISTORTION_GAIN);
            *offset += 4;
            buffer[*offset + 0] = PRESET_3_STRING_PITCH_0;
            buffer[*offset + 1] = PRESET_3_STRING_PITCH_1;
            buffer[*offset + 2] = PRESET_3_STRING_PITCH_2;
            buffer[*offset + 3] = PRESET_3_STRING_PITCH_3;
            *offset += 4;
            pack_int16(&buffer[*offset], PRESET_3_CAPO);
            *offset += 2;
            buffer[*offset + 0] = PRESET_3_PLAYING_MODE;
            *offset += 1;
            break;
    }
}

#if USE_FLASH_STORAGE

bool load_flash_data(void) {
    // Read address is different than write address
    const uint8_t *stored_data = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);

    // Validation - check magic number
    const uint8_t magic[MAGIC_NUMBER_LENGTH] = MAGIC_NUMBER;
    for (uint8_t i = 0; i < MAGIC_NUMBER_LENGTH; i++) {
        if (stored_data[i] != magic[i]) {
            return false; // Invalid data
        }
    }
    
    // Get current preset index (0-3)
    uint8_t current_preset = stored_data[OFFSET_CURRENT_PRESET];
    if (current_preset >= NUM_PRESETS) {
        current_preset = 0; // Default to preset 0 if invalid
    }
    
    // Calculate preset offset
    uint16_t preset_offset = OFFSET_PRESET_0 + (current_preset * PRESET_SIZE);
    
    // Validate preset data
    if (stored_data[preset_offset + 0] > 255) {  // Patch (0-255)
        return false; // Invalid preset data
    }
    
    // Load preset into current state
    uint16_t load_offset = preset_offset;
    unpack_preset(stored_data, &load_offset);
    
    // Load global settings separately (not part of presets)
    if (stored_data[OFFSET_VOLUME] <= 8) {  // Volume is 0-8 range
        extern void set_volume(uint8_t value);
        set_volume(stored_data[OFFSET_VOLUME]);
        extern void update_volume(void);
        update_volume();
    }
    
    // Load contrast
    if (stored_data[OFFSET_CONTRAST] <= CONTRAST_AUTO) {  // Contrast is 0-3 range
        extern void set_contrast(uint8_t value);
        set_contrast(stored_data[OFFSET_CONTRAST]);
        display_update_contrast(&display);
    }
    
    // Load lefthanded setting
    if (stored_data[OFFSET_LEFTHANDED] <= 1) {  // Lefthanded is 0-1
        extern void set_lefthanded(bool value);
        set_lefthanded(stored_data[OFFSET_LEFTHANDED] != 0);
        extern ssd1306_t display;
        extern void display_update_rotation(ssd1306_t *p);
        display_update_rotation(&display);
    }
    
    // Load global timing parameters
    extern void set_state_snapshot_window_ms(uint32_t value);
    extern void set_fret_stale_timeout_ms(uint32_t value);
    extern void set_fret_very_recent_threshold_ms(uint32_t value);
    extern void set_fret_post_strum_threshold_ms(uint32_t value);
    extern void set_fret_release_delay_ms(uint32_t value);
    set_state_snapshot_window_ms(unpack_int32(&stored_data[OFFSET_TIMING_SNAPSHOT_WINDOW]));
    set_fret_stale_timeout_ms(unpack_int32(&stored_data[OFFSET_TIMING_STALE_TIMEOUT]));
    set_fret_very_recent_threshold_ms(unpack_int32(&stored_data[OFFSET_TIMING_VERY_RECENT]));
    set_fret_post_strum_threshold_ms(unpack_int32(&stored_data[OFFSET_TIMING_POST_STRUM]));
    set_fret_release_delay_ms(unpack_int32(&stored_data[OFFSET_TIMING_RELEASE_DELAY]));
    
    return true;
}

int64_t flash_write_alarm_callback(alarm_id_t id, void *user_data) {
    (void)id;        // Unused
    (void)user_data; // Unused
    
    // Just set a flag, don't perform the actual write from interrupt context,
    // as writing is done by flash_write_task polled from the main loop.
    flash_write_pending = true;
    return 0;
}

// Get current preset index from flash
static uint8_t get_current_preset_index(void) {
    const uint8_t *stored_data = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
    const uint8_t magic[MAGIC_NUMBER_LENGTH] = MAGIC_NUMBER;
    
    // Check magic number
    for (uint8_t i = 0; i < MAGIC_NUMBER_LENGTH; i++) {
        if (stored_data[i] != magic[i]) {
            return 0; // Default to preset 0 if invalid
        }
    }
    
    uint8_t preset = stored_data[OFFSET_CURRENT_PRESET];
    if (preset < NUM_PRESETS) {
        return preset;
    }
    
    return 0; // Default to preset 0
}

// Actual flash write function
void flash_write_task(void) {
    if (!flash_write_pending || flash_write_in_progress) {
        return;
    }
    
    flash_write_in_progress = true;
    flash_write_pending = false;
    
    // Read existing flash data to preserve presets
    const uint8_t *stored_data = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
    uint8_t flash_buffer[FLASH_SECTOR_SIZE];
    
    // Initialize buffer with existing data or zeros
    const uint8_t magic[MAGIC_NUMBER_LENGTH] = MAGIC_NUMBER;
    bool has_valid_data = true;
    for (uint8_t i = 0; i < MAGIC_NUMBER_LENGTH; i++) {
        if (stored_data[i] != magic[i]) {
            has_valid_data = false;
            break;
        }
    }
    
    if (has_valid_data) {
        // Copy existing flash data (copy up to sector size, but we only use first few bytes)
        for (uint16_t i = 0; i < FLASH_SECTOR_SIZE; i++) {
            flash_buffer[i] = stored_data[i];
        }
    } else {
        // Initialize new format
        for (uint8_t i = 0; i < MAGIC_NUMBER_LENGTH; i++) {
            flash_buffer[i] = magic[i];
        }
        flash_buffer[OFFSET_VERSION] = 0xFF;
        flash_buffer[OFFSET_CURRENT_PRESET] = 0;
        
        // Initialize all presets with defaults
        uint16_t preset_offset = OFFSET_PRESET_0;
        for (uint8_t i = 0; i < NUM_PRESETS; i++) {
            pack_default_preset(flash_buffer, &preset_offset, i);
        }
        
        // Initialize global settings with current values
        extern uint8_t get_volume(void);
        extern uint8_t get_contrast(void);
        extern bool get_lefthanded(void);
        extern uint32_t get_state_snapshot_window_ms(void);
        extern uint32_t get_fret_stale_timeout_ms(void);
        extern uint32_t get_fret_very_recent_threshold_ms(void);
        extern uint32_t get_fret_post_strum_threshold_ms(void);
        extern uint32_t get_fret_release_delay_ms(void);
        flash_buffer[OFFSET_VOLUME] = get_volume();
        flash_buffer[OFFSET_CONTRAST] = get_contrast();
        flash_buffer[OFFSET_LEFTHANDED] = get_lefthanded() ? 1 : 0;
        pack_int32(&flash_buffer[OFFSET_TIMING_SNAPSHOT_WINDOW], get_state_snapshot_window_ms());
        pack_int32(&flash_buffer[OFFSET_TIMING_STALE_TIMEOUT], get_fret_stale_timeout_ms());
        pack_int32(&flash_buffer[OFFSET_TIMING_VERY_RECENT], get_fret_very_recent_threshold_ms());
        pack_int32(&flash_buffer[OFFSET_TIMING_POST_STRUM], get_fret_post_strum_threshold_ms());
        pack_int32(&flash_buffer[OFFSET_TIMING_RELEASE_DELAY], get_fret_release_delay_ms());
        
        // Fill rest with zeros. Possibly unnecessary.
        uint16_t fill_offset = OFFSET_TIMING_RELEASE_DELAY + 4;
        while (fill_offset < FLASH_SECTOR_SIZE) {
            flash_buffer[fill_offset++] = 0;
        }
    }
    
    uint8_t current_preset;
    if (pending_preset_override != 0xFF) {
        current_preset = pending_preset_override;
        pending_preset_override = 0xFF;  // Clear override after use
    } else {
        current_preset = get_current_preset_index();
    }
    
    // Update current preset index in buffer
    flash_buffer[OFFSET_CURRENT_PRESET] = current_preset;
    
    // Save current state to current preset
    uint16_t preset_offset = OFFSET_PRESET_0 + (current_preset * PRESET_SIZE);
    pack_preset(flash_buffer, &preset_offset);
    
    // Save global settings
    extern uint8_t get_volume(void);
    extern uint8_t get_contrast(void);
    extern bool get_lefthanded(void);
    extern uint32_t get_state_snapshot_window_ms(void);
    extern uint32_t get_fret_stale_timeout_ms(void);
    extern uint32_t get_fret_very_recent_threshold_ms(void);
    extern uint32_t get_fret_post_strum_threshold_ms(void);
    extern uint32_t get_fret_release_delay_ms(void);
    flash_buffer[OFFSET_VOLUME] = get_volume();
    flash_buffer[OFFSET_CONTRAST] = get_contrast();
    flash_buffer[OFFSET_LEFTHANDED] = get_lefthanded() ? 1 : 0;
    pack_int32(&flash_buffer[OFFSET_TIMING_SNAPSHOT_WINDOW], get_state_snapshot_window_ms());
    pack_int32(&flash_buffer[OFFSET_TIMING_STALE_TIMEOUT], get_fret_stale_timeout_ms());
    pack_int32(&flash_buffer[OFFSET_TIMING_VERY_RECENT], get_fret_very_recent_threshold_ms());
    pack_int32(&flash_buffer[OFFSET_TIMING_POST_STRUM], get_fret_post_strum_threshold_ms());
    pack_int32(&flash_buffer[OFFSET_TIMING_RELEASE_DELAY], get_fret_release_delay_ms());
    
    // Check if data has changed (only check the part we use, ~350 bytes + global settings)
    bool data_changed = false;
    uint16_t data_size = OFFSET_TIMING_RELEASE_DELAY + 4; // Size of all presets + header + global settings
    for (uint16_t i = 0; i < data_size; i++) {
        if (stored_data[i] != flash_buffer[i]) {
            data_changed = true;
            break;
        }
    }
    
    // If data hasn't changed and dirty flag is false, skip write
    if (!data_changed && !get_dirty()) {
        flash_write_in_progress = false;
        return;
    }
    
    // Turn on built-in LED to indicate flash write
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    
    // Disable audio to prevent issues during Core1 reset
    extern struct audio_buffer_pool *ap;
    audio_i2s_set_enabled(false);
    
    // Pause AMY synthesis to prevent issues during Core1 reset
    bool amy_was_running = amy_global.running;
    amy_global.running = 0;
    
    // Wait a bit to ensure audio is stopped and we're not in the middle of an audio buffer fill
    sleep_ms(20);
    
    // Stop audio and synth processes on core1
    multicore_reset_core1();
    
    // Small delay to ensure Core1 reset completes before flash operations
    sleep_ms(10);
    
    // Disable interrupts, write, and restore interrupts
    // Program 2 pages (512 bytes) to accommodate our 346-byte data structure
    uint32_t ints_id = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE); // Required for flash_range_program to work
    flash_range_program(FLASH_TARGET_OFFSET, flash_buffer, 2 * FLASH_PAGE_SIZE);
    restore_interrupts(ints_id);
    
    // Restart processes on core1
    multicore_launch_core1(core1_main);
    
    // Wait for Core1 ready signal with timeout handling
    int32_t ready_signal = await_message_from_other_core();
    if (ready_signal != 99) {
        // Core1 didn't respond correctly - wait a bit more and try once more
        sleep_ms(50);
        ready_signal = await_message_from_other_core();
        // If still not ready, continue anyway - Core1 will catch up
    }
    
    // Restore AMY running state
    amy_global.running = amy_was_running;
    
    // Re-enable audio after Core1 is ready
    audio_i2s_set_enabled(true);
    
    // Clear dirty flag
    set_dirty(false);
    
    // Turn off built-in LED
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    
    flash_write_in_progress = false;
}

void request_flash_write(void) {
    // Schedule writing settings to flash.
    // This delay is introduced to minimize write operations.
    if (flash_write_alarm_id) {
        cancel_alarm(flash_write_alarm_id);
    }
    flash_write_alarm_id = add_alarm_in_ms(FLASH_WRITE_DELAY_S * 1000, flash_write_alarm_callback, NULL, true);
}

// Save current state to a preset slot
void save_preset(uint8_t preset_num) {
    if (preset_num >= NUM_PRESETS) {
        return; // Invalid preset number
    }
    
    if (flash_write_in_progress) {
        return; // Don't interfere with ongoing write
    }
    
    // Set override to save to this preset
    pending_preset_override = preset_num;
    
    // Request flash write, which will use flash_write_task
    request_flash_write();
}

// Load a preset into current state
void load_preset(uint8_t preset_num) {
    if (preset_num >= NUM_PRESETS) {
        return; // Invalid preset number
    }
    
    const uint8_t *stored_data = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
    
    // Validate magic number
    const uint8_t magic[MAGIC_NUMBER_LENGTH] = MAGIC_NUMBER;
    for (uint8_t i = 0; i < MAGIC_NUMBER_LENGTH; i++) {
        if (stored_data[i] != magic[i]) {
            return; // Invalid data
        }
    }
    
    // Calculate preset offset
    uint16_t preset_offset = OFFSET_PRESET_0 + (preset_num * PRESET_SIZE);
    
    // Validate preset data
    if (stored_data[preset_offset + 0] > 255) {  // Patch (0-255)
        return; // Invalid preset data
    }
    
    // Load preset into current state
    uint16_t load_offset = preset_offset;
    unpack_preset(stored_data, &load_offset);
    
    // Update audio engine with loaded values
    extern void update_patch(void);
    extern void update_fx(amy_fx_t fx);
    extern void update_tuning(void);
    extern void update_volume(void);
    update_patch();
    update_fx(REVERB);
    update_fx(CHORUS);
    update_fx(ECHO);
    update_fx(FILTER);
    update_fx(DISTORTION);
    update_tuning();
    update_volume();
    
    // Set override to save current preset index
    pending_preset_override = preset_num;
    
    // Request flash write to update current preset index
    request_flash_write();
    
    // Update display contrast
    display_update_contrast(&display);
}

// Reset a preset to default values
void reset_preset(uint8_t preset_num) {
    if (preset_num >= NUM_PRESETS) {
        return; // Invalid preset number
    }
    
    if (flash_write_in_progress) {
        return; // Don't interfere with ongoing write
    }
    
    // Set override before loading defaults so any writes triggered by set_dirty() use correct preset
    pending_preset_override = preset_num;
    
    // Load default preset values into current state
    load_default_preset_to_state(preset_num);
    
    // Request flash write, which will use flash_write_task
    request_flash_write();
}

#else // USE_FLASH_STORAGE
// Flash storage disabled

bool load_flash_data(void) {
    return false;
}

int64_t flash_write_alarm_callback(alarm_id_t id, void *user_data) {
    (void)id;
    (void)user_data;
    return 0;
}

void request_flash_write(void) {
    return;
}

void flash_write_task(void) {
    return;
}

void save_preset(uint8_t preset_num) {
    (void)preset_num;
    return;
}

void load_preset(uint8_t preset_num) {
    (void)preset_num;
    return;
}

void reset_preset(uint8_t preset_num) {
    (void)preset_num;
    return;
}

#endif // USE_FLASH_STORAGE
