#ifndef FLASH_H_
#define FLASH_H_
#include "pico/stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

bool load_flash_data(void);
int64_t flash_write_alarm_callback(alarm_id_t id, void *user_data);
void request_flash_write(void);
void flash_write_task(void);

// Preset functions
void save_preset(uint8_t preset_num);    // Save current state to preset (0-3)
void load_preset(uint8_t preset_num);    // Load preset into current state (0-3)
void reset_preset(uint8_t preset_num);   // Reset preset to default values (0-3)

#ifdef __cplusplus
}
#endif

#endif
