#ifndef UI_H_
#define UI_H_
#include "pico/stdlib.h"
#include "ssd1306.h"

#ifdef __cplusplus
extern "C" {
#endif

void draw_rounded_rect(ssd1306_t *p, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t r);
void draw_empty_rounded_rect(ssd1306_t *p, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t r);
void draw_entry(ssd1306_t *p, uint8_t y, const char *s, bool selected);
void draw_entry_value(ssd1306_t *p, uint8_t y, const char *s, bool selected, uint8_t value);
void draw_entry_value_signed(ssd1306_t *p, uint8_t y, const char *s, bool selected, int16_t value);
void draw_entry_value_string(ssd1306_t *p, uint8_t y, const char *s, bool selected, const char *value_str);
void draw_entry_radio(ssd1306_t *p, uint8_t y, const char *s, bool selected, bool value);
void draw_entry_ab(ssd1306_t *p, uint8_t y, const char *s0, const char *s1, bool selected, bool value);
void draw_entry_pitch(ssd1306_t *p, uint8_t y, const char *s, bool selected);
uint8_t draw_multiline_string(ssd1306_t *p, uint8_t x, uint8_t y, const char *s, uint8_t line_height);

#ifdef __cplusplus
}
#endif

#endif
