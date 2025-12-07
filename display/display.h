#ifndef DISPLAY_H_
#define DISPLAY_H_
#include "pico/stdlib.h"
#include "ssd1306.h"
#include "state_data.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CONTRAST_MIN    0
#define CONTRAST_MED    1
#define CONTRAST_MAX    2
#define CONTRAST_AUTO   3

void display_init(ssd1306_t *p);
void display_draw(ssd1306_t *p);
void display_update_rotation(ssd1306_t *p);
void display_update_contrast(ssd1306_t *p);
void display_dim(ssd1306_t *p);
void display_wake(ssd1306_t *p);
void set_draw_pending(bool value);
void set_preset_save_confirmed(void);
void display_task(ssd1306_t *p);
void play_intro_animation(ssd1306_t *p, void (*callback)(void));
static inline void draw_main_screen(ssd1306_t *p, selection_t selection, context_t context);
static inline void draw_patch_screen(ssd1306_t *p, selection_t selection, context_t context);
static inline void draw_tuning_screen(ssd1306_t *p, selection_t selection, context_t context);
static inline void draw_settings_screen(ssd1306_t *p, selection_t selection, context_t context);
static inline void draw_info_screen(ssd1306_t *p, selection_t selection, context_t context);
static inline void draw_reverb_screen(ssd1306_t *p, selection_t selection, context_t context);
static inline void draw_chorus_screen(ssd1306_t *p, selection_t selection, context_t context);
static inline void draw_echo_screen(ssd1306_t *p, selection_t selection, context_t context);
static inline void draw_filter_screen(ssd1306_t *p, selection_t selection, context_t context);
static inline void draw_advanced_screen(ssd1306_t *p, selection_t selection, context_t context);
static inline void draw_distortion_screen(ssd1306_t *p, selection_t selection, context_t context);
static inline void draw_presets_screen(ssd1306_t *p, selection_t selection, context_t context);

#ifdef __cplusplus
}
#endif

#endif
