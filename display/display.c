#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "config.h"
#include "ssd1306.h"        // https://github.com/TuriSc/pico-ssd1306
#include "display.h"
#include "ui_items.h"

#include "display_strings.h"
#include "midi_note_names.h"
#include "patch_names.h"
#include "intro.h"
#include "state_data.h"
#include "icon_low_batt.h"
#include "icon_dx7.h"
#include "icon_juno_6.h"

bool draw_pending;
static bool preset_save_confirmed = false;
static int8_t last_confirmed_preset = -1;
static repeating_timer_t draw_pending_timer;
static alarm_id_t display_dim_alarm_id;

static uint8_t _calc_display_rotation() {
    #if defined (SSD1306_ROTATION)
        uint8_t base_rotation = SSD1306_ROTATION;
        uint8_t final_rotation = base_rotation;
        if (get_lefthanded()) {
            // Add 180° rotation (value 2) for left-handed mode
            final_rotation = (base_rotation + 2) % 4;
        }
        return final_rotation;
    #else
        return 0;
    #endif
}

void display_init(ssd1306_t *p) {
    p->external_vcc=false;
    ssd1306_init(p, SSD1306_WIDTH, SSD1306_HEIGHT, SSD1306_ADDRESS, I2C_PORT);

    ssd1306_set_rotation(p, _calc_display_rotation());

    display_update_contrast(p);
    ssd1306_clear(p);
    ssd1306_show(p);
}

void display_update_rotation(ssd1306_t *p) {
    ssd1306_set_rotation(p, _calc_display_rotation());
    display_draw(p);
}

void display_dim(ssd1306_t *p) {
    ssd1306_contrast(p, 1); // Minimum visible brightness
}

int64_t display_dim_callback(alarm_id_t id, void * p) {
    display_dim((ssd1306_t *)p);
    return 0;
}

void display_wake(ssd1306_t *p) {
    ssd1306_contrast(p, 255);
    if (display_dim_alarm_id) cancel_alarm(display_dim_alarm_id);
    display_dim_alarm_id = add_alarm_in_ms(DISPLAY_DIM_DELAY * 1000, display_dim_callback, p, true);
}

void display_update_contrast(ssd1306_t *p) {
    if (display_dim_alarm_id) cancel_alarm(display_dim_alarm_id);
    uint8_t contrast_setting = get_contrast();
    
    switch(contrast_setting) {
        case CONTRAST_MIN:
            ssd1306_contrast(p, 1);       // Minimum visible brightness
            break;
        case CONTRAST_MED:
            ssd1306_contrast(p, 127);     // Medium brightness
            break;
        case CONTRAST_MAX:
            ssd1306_contrast(p, 255);      // Maximum brightness
            break;
        case CONTRAST_AUTO:
            // Auto brightness - wake display and schedule dimming after inactivity
            display_wake(p);
            break;
        default:
            ssd1306_contrast(p, 255);
            break;
    }
}

void display_draw(ssd1306_t *p) {
    selection_t selection = get_selection();
    context_t context = get_context();

    ssd1306_clear(p);
    switch(context) {
        case CTX_INFO:
            draw_info_screen(p, selection, context);
        break;
        case CTX_MAIN:
            draw_main_screen(p, selection, context);
        break;
        case CTX_PATCH:
            draw_patch_screen(p, selection, context);
        break;
        case CTX_TUNING:
            draw_tuning_screen(p, selection, context);
        break;
        case CTX_SETTINGS:
            draw_settings_screen(p, selection, context);
        break;
        case CTX_REVERB:
            draw_reverb_screen(p, selection, context);
        break;
        case CTX_CHORUS:
            draw_chorus_screen(p, selection, context);
        break;
        case CTX_ECHO:
            draw_echo_screen(p, selection, context);
        break;
        case CTX_FILTER:
            draw_filter_screen(p, selection, context);
        break;
        case CTX_DISTORTION:
            draw_distortion_screen(p, selection, context);
        break;
        case CTX_ADVANCED:
            draw_advanced_screen(p, selection, context);
        break;
        case CTX_PRESETS:
            draw_presets_screen(p, selection, context);
        break;
    }

    // Low battery icon
    if(get_low_batt()) {
        // Clear an outline around the icon to avoid overlaps
        uint8_t w = 7;
        uint8_t h = 14;
        ssd1306_clear_square(p, SSD1306_HEIGHT - w - 1, SSD1306_WIDTH - h - 1, w + 1, h + 1);
        ssd1306_bmp_show_image_with_offset(p, icon_low_batt_data, icon_low_batt_size, SSD1306_HEIGHT - w, SSD1306_WIDTH - h);
    }

    ssd1306_show(p);
}

void set_draw_pending(bool value) {
    draw_pending = value;
}

void set_preset_save_confirmed(void) {
    preset_save_confirmed = true;
    last_confirmed_preset = get_preset_selected();
}

// Poll display updates from main loop
void display_task(ssd1306_t *p) {
    if (draw_pending) {
        draw_pending = false;
        display_draw(p);
    }
}

void play_intro_animation(ssd1306_t *p, void (*callback)(void)) {
    for(uint8_t current_frame=0; current_frame < INTRO_FRAMES_NUM; current_frame++) {
        ssd1306_bmp_show_image(p, intro_frames[current_frame], INTRO_FRAME_SIZE);
        ssd1306_show(p);
        busy_wait_ms(42); // About 24fps
        ssd1306_clear(p);
    }
    callback();
}

static inline void draw_main_screen(ssd1306_t *p, selection_t selection, context_t context) {
    bool selected;
    uint8_t capline_y = 0;
    uint8_t line_height = 14;

    ssd1306_draw_string(p, 2, capline_y, 1, str_main_title);
    capline_y += line_height;

    /* Patch*/
    draw_entry_value(p, capline_y, str_patch, (selection == SELECTION_PATCH), get_patch());
    capline_y += line_height;

    draw_entry_radio(p, capline_y, str_reverb, (selection == SELECTION_REVERB), get_fx(REVERB));
    capline_y += line_height;

    draw_entry_radio(p, capline_y, str_filter, (selection == SELECTION_FILTER), get_fx(FILTER));
    capline_y += line_height;

    draw_entry_radio(p, capline_y, str_chorus, (selection == SELECTION_CHORUS), get_fx(CHORUS));
    capline_y += line_height;

    draw_entry_radio(p, capline_y, str_delay, (selection == SELECTION_ECHO), get_fx(ECHO));
    capline_y += line_height;

    draw_entry_radio(p, capline_y, str_distortion, (selection == SELECTION_DISTORTION), get_fx(DISTORTION));
    capline_y += line_height;

    draw_entry(p, capline_y, str_tuning, (selection == SELECTION_TUNING));
    capline_y += line_height;

    draw_entry(p, capline_y, str_settings, (selection == SELECTION_SETTINGS));
    capline_y += line_height;
}

static inline void draw_patch_screen(ssd1306_t *p, selection_t selection, context_t context) {
    bool selected;
    uint8_t capline_y = 0;
    uint8_t line_height = 14;
    uint8_t patch = get_patch();

    ssd1306_draw_string(p, 2, capline_y, 1, str_patch);
    capline_y += line_height;

    draw_entry_value(p, capline_y, str_num, (selection == SELECTION_PATCH_NUM), patch);
    capline_y += line_height;

    if(patch < 128){
        ssd1306_bmp_show_image_with_offset(p, icon_juno_6_data, icon_juno_6_size, 1, capline_y + 2);
    } else {
        ssd1306_bmp_show_image_with_offset(p, icon_dx7_data, icon_dx7_size, 17, capline_y + 2);
    }

    capline_y += line_height + 2;
    line_height = 10;
    capline_y = draw_multiline_string(p, 0, capline_y, patch_names[patch], line_height);

    capline_y = 116;
    draw_entry(p, capline_y, str_back, (selection == SELECTION_PATCH_BACK));
}

static inline void draw_tuning_screen(ssd1306_t *p, selection_t selection, context_t context) {
    bool selected;
    uint8_t capline_y = 0;
    uint8_t line_height = 16;
    int16_t capo = get_capo();
    char str[10];

    ssd1306_draw_string(p, 2, 2, 1, str_tuning);
    capline_y += line_height;

    selection_t strings[] = {SELECTION_STRING_0, SELECTION_STRING_1, SELECTION_STRING_2, SELECTION_STRING_3};
    for(uint8_t i=0; i<4; i++) {
        uint8_t pitch = get_string_pitch(i);
        const char *str_pitch = midi_note_names[pitch % 12];
        draw_entry_pitch(p, capline_y, str_pitch, (selection == strings[i]));
        capline_y += line_height;
    }

    draw_entry_value_signed(p, capline_y, str_capo, (selection == SELECTION_CAPO), capo);
    capline_y += line_height;

    draw_entry(p, capline_y, str_reset, (selection == SELECTION_TUNING_RESET));
    capline_y += line_height * 1.5;

    capline_y = 116;
    draw_entry(p, capline_y, str_back, (selection == SELECTION_TUNING_BACK));
}

static inline void draw_settings_screen(ssd1306_t *p, selection_t selection, context_t context) {
    bool selected;
    uint8_t capline_y = 0;
    uint8_t line_height = 14;

    ssd1306_draw_string(p, 2, capline_y, 1, str_settings);
    capline_y += line_height;

    draw_entry_ab(p, capline_y, str_tapping, str_strumming, (selection == SELECTION_SETTINGS_PLAYING_MODE), get_playing_mode());
    capline_y += line_height;

    draw_entry_radio(p, capline_y, str_lefthand, (selection == SELECTION_SETTINGS_LEFTHANDED), get_lefthanded());
    capline_y += line_height;

    /* Volume */
    selected = (selection == SELECTION_SETTINGS_VOLUME);
    uint8_t volume = get_volume();
    char volume_str[16];
    
    if(volume == 0) {
        snprintf(volume_str, sizeof(volume_str), "%s off", str_volume);
    } else if(volume == 8) {
        snprintf(volume_str, sizeof(volume_str), "%s max", str_volume);
    } else {
        snprintf(volume_str, sizeof(volume_str), "%s %d", str_volume, volume);
    }
    
    if(selected) {
        draw_rounded_rect(p, 0, capline_y, 64, 12, 6);
        char value_str[8];
        if(volume == 0) {
            snprintf(value_str, sizeof(value_str), "off");
        } else if(volume == 8) {
            snprintf(value_str, sizeof(value_str), "max");
        } else {
            snprintf(value_str, sizeof(value_str), "%d", volume);
        }
        uint8_t text_x = 32 - (strlen(value_str) * 2);
        if (text_x < 8) text_x = 8;
        ssd1306_clear_string(p, 3, capline_y + 2, 1, "<");
        ssd1306_clear_string(p, text_x, capline_y + 2, 1, value_str);
        ssd1306_clear_string(p, 55, capline_y + 2, 1, ">");
    } else {
        draw_empty_rounded_rect(p, 0, capline_y, 64, 12, 6);
        ssd1306_draw_string(p, 3, capline_y + 2, 1, volume_str);
    }
    capline_y += line_height;

    /* Contrast */
    selected = (selection == SELECTION_SETTINGS_CONTRAST);
    uint8_t contrast = get_contrast();
    char contrast_str[16];
    const char *contrast_names[] = {"Min", "Med", "Max", "Auto"};
    if (contrast <= 3) {
        snprintf(contrast_str, sizeof(contrast_str), "%s %s", str_contrast, contrast_names[contrast]);
    } else {
        snprintf(contrast_str, sizeof(contrast_str), "%s ?", str_contrast);
    }
    
    if(selected) {
        draw_rounded_rect(p, 0, capline_y, 64, 12, 6);
        char value_str[8];
        if (contrast <= 3) {
            snprintf(value_str, sizeof(value_str), "%s", contrast_names[contrast]);
        } else {
            snprintf(value_str, sizeof(value_str), "?");
        }
        uint8_t text_x = 32 - (strlen(value_str) * 2);
        if (text_x < 8) text_x = 8;
        ssd1306_clear_string(p, 3, capline_y + 2, 1, "<");
        ssd1306_clear_string(p, text_x, capline_y + 2, 1, value_str);
        ssd1306_clear_string(p, 55, capline_y + 2, 1, ">");
    } else {
        draw_empty_rounded_rect(p, 0, capline_y, 64, 12, 6);
        ssd1306_draw_string(p, 3, capline_y + 2, 1, contrast_str);
    }
    capline_y += line_height;

    draw_entry(p, capline_y, str_advanced, (selection == SELECTION_SETTINGS_ADVANCED));
    capline_y += line_height;

    draw_entry(p, capline_y, str_info, (selection == SELECTION_SETTINGS_INFO));

    capline_y = 116;

    draw_entry(p, capline_y, str_back, (selection == SELECTION_SETTINGS_BACK));
}

static inline void draw_info_screen(ssd1306_t *p, selection_t selection, context_t context) {
    uint8_t capline_y = 0;
    char version_number[18];
    uint8_t line_height = 10;

    snprintf(version_number, sizeof(version_number), "%s%s", str_version, PROGRAM_VERSION);
    ssd1306_draw_string(p, 0, capline_y, 1, version_number);
    capline_y += line_height;
    capline_y += line_height;

    capline_y = draw_multiline_string(p, 0, capline_y, PROGRAM_URL, line_height);

    capline_y = 116;
    draw_entry(p, capline_y, str_back, (selection == SELECTION_INFO_BACK));
}

static inline void draw_advanced_screen(ssd1306_t *p, selection_t selection, context_t context) {
    uint8_t capline_y = 0;
    uint8_t line_height = 14;
    char value_str[16];

    ssd1306_draw_string(p, 2, capline_y, 1, str_advanced);
    capline_y += line_height;

    uint32_t snapshot_window = get_state_snapshot_window_ms();
    snprintf(value_str, sizeof(value_str), "%ums", (unsigned int)snapshot_window);
    draw_entry_value_string(p, capline_y, str_snapshot_window, (selection == SELECTION_ADVANCED_SNAPSHOT_WINDOW), value_str);
    capline_y += line_height;

    uint32_t stale_timeout = get_fret_stale_timeout_ms();
    snprintf(value_str, sizeof(value_str), "%ums", (unsigned int)stale_timeout);
    draw_entry_value_string(p, capline_y, str_stale_timeout, (selection == SELECTION_ADVANCED_STALE_TIMEOUT), value_str);
    capline_y += line_height;

    uint32_t very_recent = get_fret_very_recent_threshold_ms();
    snprintf(value_str, sizeof(value_str), "%ums", (unsigned int)very_recent);
    draw_entry_value_string(p, capline_y, str_very_recent, (selection == SELECTION_ADVANCED_VERY_RECENT), value_str);
    capline_y += line_height;

    uint32_t post_strum = get_fret_post_strum_threshold_ms();
    snprintf(value_str, sizeof(value_str), "%ums", (unsigned int)post_strum);
    draw_entry_value_string(p, capline_y, str_post_strum, (selection == SELECTION_ADVANCED_POST_STRUM), value_str);
    capline_y += line_height;

    uint32_t release_delay = get_fret_release_delay_ms();
    snprintf(value_str, sizeof(value_str), "%ums", (unsigned int)release_delay);
    draw_entry_value_string(p, capline_y, str_release_delay, (selection == SELECTION_ADVANCED_RELEASE_DELAY), value_str);
    capline_y += line_height;

    draw_entry(p, capline_y, str_presets, (selection == SELECTION_ADVANCED_PRESETS));
    capline_y += line_height;

    draw_entry(p, capline_y, str_reset, (selection == SELECTION_ADVANCED_RESET));
    capline_y += line_height * 1.5;

    capline_y = 116;
    draw_entry(p, capline_y, str_back, (selection == SELECTION_ADVANCED_BACK));
}

static inline void draw_presets_screen(ssd1306_t *p, selection_t selection, context_t context) {
    uint8_t capline_y = 0;
    uint8_t line_height = 14;
    char value_str[16];
    int8_t preset = get_preset_selected();

    // Clear confirmation if preset selection changed
    if (preset != last_confirmed_preset) {
        preset_save_confirmed = false;
    }

    ssd1306_draw_string(p, 2, capline_y, 1, str_presets);
    capline_y += line_height;

    // Save entry with preset number or "OK" if save was confirmed for current preset
    if (preset_save_confirmed && preset == last_confirmed_preset) {
        snprintf(value_str, sizeof(value_str), "OK");
    } else if (preset == -1) {
        snprintf(value_str, sizeof(value_str), "-");
    } else {
        snprintf(value_str, sizeof(value_str), "%d", preset + 1); // Display 1-4
    }
    draw_entry_value_string(p, capline_y, str_save, (selection == SELECTION_PRESETS_SAVE), value_str);
    capline_y += line_height;

    // Load entry with preset number
    if (preset == -1) {
        snprintf(value_str, sizeof(value_str), "-");
    } else {
        snprintf(value_str, sizeof(value_str), "%d", preset + 1); // Display 1-4
    }
    draw_entry_value_string(p, capline_y, str_load, (selection == SELECTION_PRESETS_LOAD), value_str);
    capline_y += line_height;

    draw_entry(p, capline_y, str_reset, (selection == SELECTION_PRESETS_RESET));
    capline_y += line_height;

    capline_y = 116;
    draw_entry(p, capline_y, str_back, (selection == SELECTION_PRESETS_BACK));
}

static inline void draw_reverb_screen(ssd1306_t *p, selection_t selection, context_t context) {
    uint8_t capline_y = 0;
    uint8_t line_height = 14;
    char value_str[16];

    ssd1306_draw_string(p, 2, capline_y, 1, str_reverb);
    capline_y += line_height;

    draw_entry_ab(p, capline_y, str_off, str_on, (selection == SELECTION_REVERB_ONOFF), get_fx(REVERB));
    capline_y += line_height;

    float liveness = get_reverb_liveness();
    snprintf(value_str, sizeof(value_str), "%.2f", liveness);
    draw_entry_value_string(p, capline_y, str_liveness, (selection == SELECTION_REVERB_LIVENESS), value_str);
    capline_y += line_height;

    float damping = get_reverb_damping();
    snprintf(value_str, sizeof(value_str), "%.2f", damping);
    draw_entry_value_string(p, capline_y, str_damping, (selection == SELECTION_REVERB_DAMPING), value_str);
    capline_y += line_height;

    float xover = get_reverb_xover_hz();
    snprintf(value_str, sizeof(value_str), "%.0fHz", xover);
    draw_entry_value_string(p, capline_y, str_xover, (selection == SELECTION_REVERB_XOVER), value_str);
    capline_y += line_height;

    draw_entry(p, capline_y, str_reset, (selection == SELECTION_REVERB_RESET));
    capline_y += line_height * 1.5;

    capline_y = 116;
    draw_entry(p, capline_y, str_back, (selection == SELECTION_REVERB_BACK));
}

static inline void draw_chorus_screen(ssd1306_t *p, selection_t selection, context_t context) {
    uint8_t capline_y = 0;
    uint8_t line_height = 14;
    char value_str[16];

    ssd1306_draw_string(p, 2, capline_y, 1, str_chorus);
    capline_y += line_height;

    draw_entry_ab(p, capline_y, str_off, str_on, (selection == SELECTION_CHORUS_ONOFF), get_fx(CHORUS));
    capline_y += line_height;

    int max_delay = get_chorus_max_delay();
    snprintf(value_str, sizeof(value_str), "%d", max_delay);
    draw_entry_value_string(p, capline_y, str_max_delay, (selection == SELECTION_CHORUS_MAX_DELAY), value_str);
    capline_y += line_height;

    float lfo_freq = get_chorus_lfo_freq();
    snprintf(value_str, sizeof(value_str), "%.2fHz", lfo_freq);
    draw_entry_value_string(p, capline_y, str_lfo_freq, (selection == SELECTION_CHORUS_LFO_FREQ), value_str);
    capline_y += line_height;

    float depth = get_chorus_depth();
    snprintf(value_str, sizeof(value_str), "%.2f", depth);
    draw_entry_value_string(p, capline_y, str_depth, (selection == SELECTION_CHORUS_DEPTH), value_str);
    capline_y += line_height;

    draw_entry(p, capline_y, str_reset, (selection == SELECTION_CHORUS_RESET));
    capline_y += line_height * 1.5;

    capline_y = 116;
    draw_entry(p, capline_y, str_back, (selection == SELECTION_CHORUS_BACK));
}

static inline void draw_echo_screen(ssd1306_t *p, selection_t selection, context_t context) {
    uint8_t capline_y = 0;
    uint8_t line_height = 14;
    char value_str[16];

    ssd1306_draw_string(p, 2, capline_y, 1, str_delay);
    capline_y += line_height;

    draw_entry_ab(p, capline_y, str_off, str_on, (selection == SELECTION_ECHO_ONOFF), get_fx(ECHO));
    capline_y += line_height;

    float delay_ms = get_echo_delay_ms();
    snprintf(value_str, sizeof(value_str), "%.0fms", delay_ms);
    draw_entry_value_string(p, capline_y, str_delay_ms, (selection == SELECTION_ECHO_DELAY), value_str);
    capline_y += line_height;

    float feedback = get_echo_feedback();
    snprintf(value_str, sizeof(value_str), "%.2f", feedback);
    draw_entry_value_string(p, capline_y, str_feedback, (selection == SELECTION_ECHO_FEEDBACK), value_str);
    capline_y += line_height;

    float filter_coef = get_echo_filter_coef();
    snprintf(value_str, sizeof(value_str), "%.2f", filter_coef);
    draw_entry_value_string(p, capline_y, str_filter_coef, (selection == SELECTION_ECHO_FILTER), value_str);
    capline_y += line_height;

    draw_entry(p, capline_y, str_reset, (selection == SELECTION_ECHO_RESET));
    capline_y += line_height * 1.5;

    capline_y = 116;
    draw_entry(p, capline_y, str_back, (selection == SELECTION_ECHO_BACK));
}

static inline void draw_filter_screen(ssd1306_t *p, selection_t selection, context_t context) {
    uint8_t capline_y = 0;
    uint8_t line_height = 14;
    char value_str[16];

    ssd1306_draw_string(p, 2, capline_y, 1, str_filter);
    capline_y += line_height;

    draw_entry_ab(p, capline_y, str_off, str_on, (selection == SELECTION_FILTER_ONOFF), get_fx(FILTER));
    capline_y += line_height;

    float freq_hz = get_filter_freq_hz();
    if (freq_hz >= 1000.0f) {
        snprintf(value_str, sizeof(value_str), "%.1fkHz", freq_hz / 1000.0f);
    } else {
        snprintf(value_str, sizeof(value_str), "%.0fHz", freq_hz);
    }
    draw_entry_value_string(p, capline_y, str_freq, (selection == SELECTION_FILTER_FREQ), value_str);
    capline_y += line_height;

    float resonance = get_filter_resonance();
    snprintf(value_str, sizeof(value_str), "%.2f", resonance);
    draw_entry_value_string(p, capline_y, str_resonance, (selection == SELECTION_FILTER_RESONANCE), value_str);
    capline_y += line_height;

    draw_entry(p, capline_y, str_reset, (selection == SELECTION_FILTER_RESET));
    capline_y += line_height * 1.5;

    capline_y = 116;
    draw_entry(p, capline_y, str_back, (selection == SELECTION_FILTER_BACK));
}

static inline void draw_distortion_screen(ssd1306_t *p, selection_t selection, context_t context) {
    uint8_t capline_y = 0;
    uint8_t line_height = 14;
    char value_str[16];

    ssd1306_draw_string(p, 2, capline_y, 1, str_distortion);
    capline_y += line_height;

    draw_entry_ab(p, capline_y, str_off, str_on, (selection == SELECTION_DISTORTION_ONOFF), get_fx(DISTORTION));
    capline_y += line_height;

    float level = get_distortion_level();
    snprintf(value_str, sizeof(value_str), "%.2f", level);
    draw_entry_value_string(p, capline_y, str_level, (selection == SELECTION_DISTORTION_LEVEL), value_str);
    capline_y += line_height;

    // Gain (display as 1.0-2.0, stored internally as 10.0-20.0)
    float gain = get_distortion_gain() / 10.0f;
    snprintf(value_str, sizeof(value_str), "%.1fx", gain);
    draw_entry_value_string(p, capline_y, str_gain, (selection == SELECTION_DISTORTION_GAIN), value_str);
    capline_y += line_height;

    draw_entry(p, capline_y, str_reset, (selection == SELECTION_DISTORTION_RESET));
    capline_y += line_height * 1.5;

    capline_y = 116;
    draw_entry(p, capline_y, str_back, (selection == SELECTION_DISTORTION_BACK));
}
