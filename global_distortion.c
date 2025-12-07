#include "global_distortion.h"
#include "state_data.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// Distortion state
static struct {
    bool enabled;
    float level;  // 0.0 to 1.0 - amount of distortion
    float gain;   // 10.0 to 20.0 internally (displayed as 1.0 to 2.0 in UI) - drive/gain before distortion
} distortion_state;

// Hard clipping with asymmetric character
static inline float hard_clip(float x, float threshold) {
    if (x > threshold) return threshold;
    if (x < -threshold) return -threshold;
    return x;
}

// Harsh waveshaper using polynomial for aggressive distortion
static inline float harsh_waveshape(float x) {
    // Clamp to prevent overflow
    if (x > 1.0f) return 1.0f;
    if (x < -1.0f) return -1.0f;
    
    // Asymmetric waveshaping
    float abs_x = x < 0.0f ? -x : x;
    float sign = x < 0.0f ? -1.0f : 1.0f;
    
    // Polynomial waveshaping: x^3 for harsh distortion
    float shaped = abs_x * abs_x * abs_x;
    
    return sign * shaped;
}

void global_distortion_init(void) {
    distortion_state.enabled = false;
    distortion_state.level = 0.0f;
    distortion_state.gain = 1.0f;
}

void config_global_distortion(float level, float gain) {
    // Clamp level to 0.0-1.0
    if (level < 0.0f) level = 0.0f;
    if (level > 1.0f) level = 1.0f;
    
    // Clamp gain to 10.0-20.0 (internal range, displayed as 1.0-2.0 in UI)
    if (gain < 10.0f) gain = 10.0f;
    if (gain > 20.0f) gain = 20.0f;
    
    distortion_state.level = level;
    distortion_state.gain = gain;
}

void global_distortion_set_enabled(bool enabled) {
    distortion_state.enabled = enabled;
}

SAMPLE global_distortion_process(int16_t *buffer, uint16_t length) {
    // Check if distortion is enabled and level > 0
    if (!distortion_state.enabled || distortion_state.level <= 0.0f) {
        return 0;
    }

    SAMPLE max_val = 0;
    const float SAMPLE_MAX_F = 32767.0f;
    const float SAMPLE_MIN_F = -32768.0f;
    
    // Process each sample in the interleaved buffer
    for (uint16_t i = 0; i < length * AMY_NCHANS; i++) {
        // Convert int16 to float (-1.0 to 1.0 range)
        float clean = (float)buffer[i] / SAMPLE_MAX_F;
        
        // Gain controls drive amount (how hard we push the signal)
        // Level controls mix between clean and distorted (0.0 = clean, 1.0 = fully distorted)
        float drive = distortion_state.gain;
        
        float driven = clean * drive;
        
        float waveshaped = harsh_waveshape(driven);
        
        // Apply hard clipping for even harsher character
        // Clipping threshold decreases as gain increases for more aggressive sound
        float clip_threshold = 1.0f / (1.0f + (drive - 1.0f) * 0.5f);
        float distorted = hard_clip(waveshaped, clip_threshold);
        
        // Normalize to maintain approximately constant volume
        // The waveshaping and clipping reduce volume, so we compensate
        float volume_compensation = 1.0f / (1.0f + (drive - 1.0f) * 0.4f);
        float normalized_distorted = distorted * volume_compensation;
        
        // Mix clean and distorted based on level
        // level = 0.0: all clean, level = 1.0: all distorted
        float output_f = clean * (1.0f - distortion_state.level) + normalized_distorted * distortion_state.level;
        
        // Convert back to int16 range
        float output = output_f * SAMPLE_MAX_F;
        
        // Hard clip to prevent overflow
        if (output > SAMPLE_MAX_F) output = SAMPLE_MAX_F;
        if (output < SAMPLE_MIN_F) output = SAMPLE_MIN_F;
        
        buffer[i] = (int16_t)output;
        
        // Track max value
        int16_t abs_val = (output < 0) ? -output : output;
        if (abs_val > max_val) max_val = abs_val;
    }
    
    return max_val;
}