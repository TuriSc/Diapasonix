#ifndef TOUCH_H_
#define TOUCH_H_
#include "pico/stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

void mpr121_initialize();
void mpr121_task();

void touch_on(uint8_t id);
void touch_off(uint8_t id);

extern void note_on(uint8_t string, uint8_t note);
extern void note_off(uint8_t string, uint8_t note);

uint16_t get_touched();

#ifdef __cplusplus
}
#endif

#endif