#ifndef GLOBAL_DISTORTION_H_
#define GLOBAL_DISTORTION_H_

#include "amy.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialize global distortion
void global_distortion_init(void);

// Configure global distortion
// level: 0.0 to 1.0 (amount of distortion effect)
// gain: 1.0 to 5.0+ (drive/gain before distortion)
void config_global_distortion(float level, float gain);

// Enable/disable global distortion
void global_distortion_set_enabled(bool enabled);

// Process audio buffer through global distortion
// Returns max sample value after distortion
SAMPLE global_distortion_process(int16_t *buffer, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* GLOBAL_DISTORTION_H_ */

