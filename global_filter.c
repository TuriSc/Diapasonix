#include "global_filter.h"
#include "amy.h"
#include "state_data.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// Constants from filters.c
#define LOWEST_RATIO 0.0001
#define FILT_NUM_DELAYS  4

static global_filter_state_t filter_state[AMY_NCHANS];  // Separate state for each channel

// Helper functions from filters.c (we'll use AMY's filter coefficient generation)
extern int8_t dsps_biquad_gen_lpf_f32(SAMPLE *coeffs, float f, float qFactor);
extern SAMPLE dsps_biquad_f32_ansi_split_fb_twice(const SAMPLE *input, SAMPLE *output, int len, SAMPLE *coef, SAMPLE *w, SAMPLE max_val);
extern SAMPLE scan_max(SAMPLE *block, int len);

void global_filter_init(void) {
    for (int c = 0; c < AMY_NCHANS; c++) {
        memset(&filter_state[c], 0, sizeof(global_filter_state_t));
        filter_state[c].filter_freq_hz = 1000.0f;
        filter_state[c].filter_resonance = 0.7f;
        filter_state[c].enabled = false;
        filter_state[c].last_filt_norm_bits = 0;
        for (int i = 0; i < 8; i++) {
            filter_state[c].filter_delay[i] = 0;
        }
    }
}

void config_global_filter(float freq_hz, float resonance) {
    for (int c = 0; c < AMY_NCHANS; c++) {
        filter_state[c].filter_freq_hz = freq_hz;
        filter_state[c].filter_resonance = resonance;
    }
}

void global_filter_set_enabled(bool enabled) {
    for (int c = 0; c < AMY_NCHANS; c++) {
        filter_state[c].enabled = enabled;
        if (!enabled) {
            // Reset filter state when disabled
            for (int i = 0; i < 8; i++) {
                filter_state[c].filter_delay[i] = 0;
            }
            filter_state[c].last_filt_norm_bits = 0;
        }
    }
}

SAMPLE global_filter_process(int16_t *buffer, uint16_t length) {
    // Check if filter is enabled
    if (!filter_state[0].enabled) {
        return 0;
    }

    SAMPLE max_val = 0;

    // Process each channel separately
    // Buffer is interleaved: [L0, R0, L1, R1, L2, R2, ...]
    for (int16_t c = 0; c < AMY_NCHANS; c++) {
        // Extract channel samples from interleaved buffer
        SAMPLE channel_samples[AMY_BLOCK_SIZE];
        for (int16_t i = 0; i < length; i++) {
            channel_samples[i] = buffer[AMY_NCHANS * i + c];
        }
        
        // Calculate filter coefficient ratio
        float ratio = filter_state[c].filter_freq_hz / (float)AMY_SAMPLE_RATE;
        if (ratio < LOWEST_RATIO) ratio = LOWEST_RATIO;
        if (ratio > 0.45f) ratio = 0.45f;  // Prevent aliasing

        // Generate LPF coefficients (always LPF24)
        SAMPLE coeffs[5];
        dsps_biquad_gen_lpf_f32(coeffs, ratio, filter_state[c].filter_resonance);

        // Calculate max value for normalization
        SAMPLE chan_max_val = scan_max(channel_samples, length);
        SAMPLE filtmax = scan_max(filter_state[c].filter_delay, 2 * FILT_NUM_DELAYS);
        
        if (chan_max_val == 0 && filtmax == 0) continue;

        // Apply LPF24 filter (24 dB/oct by running the same filter twice)
        chan_max_val = dsps_biquad_f32_ansi_split_fb_twice(channel_samples, channel_samples, length, coeffs, filter_state[c].filter_delay, chan_max_val);

        // Write filtered samples back to interleaved buffer
        for (int16_t i = 0; i < length; i++) {
            buffer[AMY_NCHANS * i + c] = channel_samples[i];
        }

        if (chan_max_val > max_val) max_val = chan_max_val;
    }

    return max_val;
}

