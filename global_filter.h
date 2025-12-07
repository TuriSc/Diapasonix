#ifndef GLOBAL_FILTER_H_
#define GLOBAL_FILTER_H_

#include "amy.h"

#ifdef __cplusplus
extern "C" {
#endif

// Global filter state structure
typedef struct global_filter_state {
    SAMPLE filter_delay[8];  // Filter delay line (FILT_NUM_DELAYS * 2 = 4 * 2 = 8)
    int last_filt_norm_bits;
    float filter_freq_hz;    // Filter cutoff frequency in Hz
    float filter_resonance;  // Filter Q factor
    bool enabled;
} global_filter_state_t;

void global_filter_init(void);
void config_global_filter(float freq_hz, float resonance); // LPF24 only
void global_filter_set_enabled(bool enabled);

// Process audio buffer through global filter
// Returns max sample value after filtering
SAMPLE global_filter_process(int16_t *buffer, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* GLOBAL_FILTER_H_ */

