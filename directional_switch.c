#include "pico/stdlib.h"
#include "config.h"
#include "directional_switch.h"
#include "state_data.h"
#include "display/display.h"
#include "flash.h"
#include "ssd1306.h"

extern void update_display();
extern void update_patch();
extern void update_tuning();
extern void update_fx(amy_fx_t fx);
extern void update_volume();
extern ssd1306_t display;

static inline void wake_display_if_auto() {
    if (get_contrast() == CONTRAST_AUTO) {
        display_wake(&display);
    }
}

// Polling-based button state
static bool last_button_state[5] = {true, true, true, true, true}; // Pull-up = high when not pressed
static uint32_t button_press_time[5] = {0};
static bool long_press_triggered[5] = {false, false, false, false, false}; // Track if long press already triggered
static const uint8_t button_pins[5] = {DS_A, DS_B, DS_C, DS_D, DS_X};

static inline void ds_up() {
    wake_display_if_auto();
    set_selection_up();
}

static inline void ds_down() {
    wake_display_if_auto();
    set_selection_down();
}

static inline void ds_left() {
    wake_display_if_auto();
    selection_t selection = get_selection();
    context_t context = get_context();
    
    switch (context) {
        case CTX_REVERB:
            switch(selection) {
                case SELECTION_REVERB_LIVENESS:
                    set_reverb_liveness_down();
                    update_fx(REVERB);
                    set_draw_pending(true);
                    break;
                case SELECTION_REVERB_DAMPING:
                    set_reverb_damping_down();
                    update_fx(REVERB);
                    set_draw_pending(true);
                    break;
                case SELECTION_REVERB_XOVER:
                    set_reverb_xover_hz_down();
                    update_fx(REVERB);
                    set_draw_pending(true);
                    break;
            }
            break;
        case CTX_CHORUS:
            switch(selection) {
                case SELECTION_CHORUS_MAX_DELAY:
                    set_chorus_max_delay_down();
                    update_fx(CHORUS);
                    set_draw_pending(true);
                    break;
                case SELECTION_CHORUS_LFO_FREQ:
                    set_chorus_lfo_freq_down();
                    update_fx(CHORUS);
                    set_draw_pending(true);
                    break;
                case SELECTION_CHORUS_DEPTH:
                    set_chorus_depth_down();
                    update_fx(CHORUS);
                    set_draw_pending(true);
                    break;
            }
            break;
        case CTX_ECHO:
            switch(selection) {
                case SELECTION_ECHO_DELAY:
                    set_echo_delay_ms_down();
                    update_fx(ECHO);
                    set_draw_pending(true);
                    break;
                case SELECTION_ECHO_FEEDBACK:
                    set_echo_feedback_down();
                    update_fx(ECHO);
                    set_draw_pending(true);
                    break;
                case SELECTION_ECHO_FILTER:
                    set_echo_filter_coef_down();
                    update_fx(ECHO);
                    set_draw_pending(true);
                    break;
            }
            break;
        case CTX_FILTER:
            switch(selection) {
                case SELECTION_FILTER_FREQ:
                    set_filter_freq_hz_down();
                    update_fx(FILTER);
                    set_draw_pending(true);
                    break;
                case SELECTION_FILTER_RESONANCE:
                    set_filter_resonance_down();
                    update_fx(FILTER);
                    set_draw_pending(true);
                    break;
            }
            break;
        case CTX_DISTORTION:
            switch(selection) {
                case SELECTION_DISTORTION_LEVEL:
                    set_distortion_level_down();
                    set_draw_pending(true);
                    break;
                case SELECTION_DISTORTION_GAIN:
                    set_distortion_gain_down();
                    set_draw_pending(true);
                    break;
            }
            break;
        default:
            switch (selection) {
                case SELECTION_PATCH:
                case SELECTION_PATCH_NUM:
                    set_patch_down();
                    update_patch();
                break;
                case SELECTION_CAPO:
                    set_capo_down();
                    update_tuning();
                break;
                case SELECTION_STRING_0:
                    set_string_pitch_down(0);
                    update_tuning();
                break;
                case SELECTION_STRING_1:
                    set_string_pitch_down(1);
                    update_tuning();
                break;
                case SELECTION_STRING_2:
                    set_string_pitch_down(2);
                    update_tuning();
                break;
                case SELECTION_STRING_3:
                    set_string_pitch_down(3);
                    update_tuning();
                break;
                case SELECTION_SETTINGS_VOLUME:
                    set_volume_down();
                    update_volume();
                    set_draw_pending(true);
                break;
                case SELECTION_SETTINGS_CONTRAST:
                    set_contrast_down();
                    display_update_contrast(&display);
                    set_draw_pending(true);
                break;
            }
            break;
        case CTX_ADVANCED:
            switch(selection) {
                case SELECTION_ADVANCED_SNAPSHOT_WINDOW:
                    set_state_snapshot_window_ms_down();
                    set_draw_pending(true);
                    break;
                case SELECTION_ADVANCED_STALE_TIMEOUT:
                    set_fret_stale_timeout_ms_down();
                    set_draw_pending(true);
                    break;
                case SELECTION_ADVANCED_VERY_RECENT:
                    set_fret_very_recent_threshold_ms_down();
                    set_draw_pending(true);
                    break;
                case SELECTION_ADVANCED_POST_STRUM:
                    set_fret_post_strum_threshold_ms_down();
                    set_draw_pending(true);
                    break;
                case SELECTION_ADVANCED_RELEASE_DELAY:
                    set_fret_release_delay_ms_down();
                    set_draw_pending(true);
                    break;
            }
            break;
        case CTX_PRESETS:
            switch(selection) {
                case SELECTION_PRESETS_SAVE:
                case SELECTION_PRESETS_LOAD:
                    {
                    // Decrease preset number (wrap around from 0 to -1/null, then to 3)
                    int8_t preset = get_preset_selected();
                    if (preset == -1) {
                        preset = 3;
                    } else if (preset == 0) {
                        preset = -1;
                    } else {
                        preset--;
                    }
                    set_preset_selected(preset);
                    // If Load is selected and new preset is valid, load it instantly
                    if (selection == SELECTION_PRESETS_LOAD && preset >= 0) {
                        load_preset(preset);
                    }
                    set_draw_pending(true);
                    }
                    break;
            }
            break;
    }
}

static inline void ds_right() {
    wake_display_if_auto();
    selection_t selection = get_selection();
    context_t context = get_context();
    
    switch (context) {
        case CTX_REVERB:
            switch(selection) {
                case SELECTION_REVERB_LIVENESS:
                    set_reverb_liveness_up();
                    update_fx(REVERB);
                    set_draw_pending(true);
                    break;
                case SELECTION_REVERB_DAMPING:
                    set_reverb_damping_up();
                    update_fx(REVERB);
                    set_draw_pending(true);
                    break;
                case SELECTION_REVERB_XOVER:
                    set_reverb_xover_hz_up();
                    update_fx(REVERB);
                    set_draw_pending(true);
                    break;
            }
            break;
        case CTX_CHORUS:
            switch(selection) {
                case SELECTION_CHORUS_MAX_DELAY:
                    set_chorus_max_delay_up();
                    update_fx(CHORUS);
                    set_draw_pending(true);
                    break;
                case SELECTION_CHORUS_LFO_FREQ:
                    set_chorus_lfo_freq_up();
                    update_fx(CHORUS);
                    set_draw_pending(true);
                    break;
                case SELECTION_CHORUS_DEPTH:
                    set_chorus_depth_up();
                    update_fx(CHORUS);
                    set_draw_pending(true);
                    break;
            }
            break;
        case CTX_ECHO:
            switch(selection) {
                case SELECTION_ECHO_DELAY:
                    set_echo_delay_ms_up();
                    update_fx(ECHO);
                    set_draw_pending(true);
                    break;
                case SELECTION_ECHO_FEEDBACK:
                    set_echo_feedback_up();
                    update_fx(ECHO);
                    set_draw_pending(true);
                    break;
                case SELECTION_ECHO_FILTER:
                    set_echo_filter_coef_up();
                    update_fx(ECHO);
                    set_draw_pending(true);
                    break;
            }
            break;
        case CTX_FILTER:
            switch(selection) {
                case SELECTION_FILTER_FREQ:
                    set_filter_freq_hz_up();
                    update_fx(FILTER);
                    set_draw_pending(true);
                    break;
                case SELECTION_FILTER_RESONANCE:
                    set_filter_resonance_up();
                    update_fx(FILTER);
                    set_draw_pending(true);
                    break;
            }
            break;
        case CTX_DISTORTION:
            switch(selection) {
                case SELECTION_DISTORTION_LEVEL:
                    set_distortion_level_up();
                    set_draw_pending(true);
                    break;
                case SELECTION_DISTORTION_GAIN:
                    set_distortion_gain_up();
                    set_draw_pending(true);
                    break;
            }
            break;
        default:
            switch (selection) {
                case SELECTION_PATCH:
                case SELECTION_PATCH_NUM:
                    set_patch_up();
                    update_patch();
                break;
                case SELECTION_CAPO:
                    set_capo_up();
                    update_tuning();
                break;
                case SELECTION_STRING_0:
                    set_string_pitch_up(0);
                    update_tuning();
                break;
                case SELECTION_STRING_1:
                    set_string_pitch_up(1);
                    update_tuning();
                break;
                case SELECTION_STRING_2:
                    set_string_pitch_up(2);
                    update_tuning();
                break;
                case SELECTION_STRING_3:
                    set_string_pitch_up(3);
                    update_tuning();
                break;
                case SELECTION_SETTINGS_VOLUME:
                    set_volume_up();
                    update_volume();
                    set_draw_pending(true);
                break;
                case SELECTION_SETTINGS_CONTRAST:
                    set_contrast_up();
                    display_update_contrast(&display);
                    set_draw_pending(true);
                break;
            }
            break;
        case CTX_ADVANCED:
            switch(selection) {
                case SELECTION_ADVANCED_SNAPSHOT_WINDOW:
                    set_state_snapshot_window_ms_up();
                    set_draw_pending(true);
                    break;
                case SELECTION_ADVANCED_STALE_TIMEOUT:
                    set_fret_stale_timeout_ms_up();
                    set_draw_pending(true);
                    break;
                case SELECTION_ADVANCED_VERY_RECENT:
                    set_fret_very_recent_threshold_ms_up();
                    set_draw_pending(true);
                    break;
                case SELECTION_ADVANCED_POST_STRUM:
                    set_fret_post_strum_threshold_ms_up();
                    set_draw_pending(true);
                    break;
                case SELECTION_ADVANCED_RELEASE_DELAY:
                    set_fret_release_delay_ms_up();
                    set_draw_pending(true);
                    break;
            }
            break;
        case CTX_PRESETS:
            switch(selection) {
                case SELECTION_PRESETS_SAVE:
                case SELECTION_PRESETS_LOAD:
                    {
                    // Increase preset number (wrap around from 3 to -1/null, then to 0)
                    int8_t preset = get_preset_selected();
                    if (preset == -1) {
                        preset = 0;
                    } else if (preset == 3) {
                        preset = -1;
                    } else {
                        preset++;
                    }
                    set_preset_selected(preset);
                    // If Load is selected and preset is valid, load it instantly
                    if (selection == SELECTION_PRESETS_LOAD && preset >= 0) {
                        load_preset(preset);
                    }
                    set_draw_pending(true);
                    }
                    break;
            }
            break;
    }
}

static inline void ds_center() {
    wake_display_if_auto();
    context_t context = get_context();
    selection_t selection = get_selection();

    switch (context) {
        case CTX_INFO:
            set_context(CTX_SETTINGS);
            set_selection(SELECTION_SETTINGS_INFO);
            return;
        break;
    }

    switch(selection){
        case SELECTION_PATCH:
            set_context(CTX_PATCH);
            set_selection(SELECTION_PATCH_NUM);
        break;
        case SELECTION_REVERB:
            set_fx(REVERB, !get_fx(REVERB));
            update_fx(REVERB);
        break;
        case SELECTION_FILTER:
            set_fx(FILTER, !get_fx(FILTER));
            update_fx(FILTER);
        break;
        case SELECTION_CHORUS:
            set_fx(CHORUS, !get_fx(CHORUS));
            update_fx(CHORUS);
        break;
        case SELECTION_ECHO:
            set_fx(ECHO, !get_fx(ECHO));
            update_fx(ECHO);
        break;
        case SELECTION_DISTORTION:
            set_fx(DISTORTION, !get_fx(DISTORTION));
            set_draw_pending(true);
        break;
        case SELECTION_TUNING:
            set_context(CTX_TUNING);
            set_selection(SELECTION_STRING_0);
        break;
        case SELECTION_SETTINGS:
            set_context(CTX_SETTINGS);
            set_selection(SELECTION_SETTINGS_PLAYING_MODE);
        break;
        case SELECTION_SETTINGS_ADVANCED:
            set_context(CTX_ADVANCED);
            set_selection(SELECTION_ADVANCED_SNAPSHOT_WINDOW);
        break;
        case SELECTION_ADVANCED_PRESETS:
            set_context(CTX_PRESETS);
            set_selection(SELECTION_PRESETS_SAVE);
            set_preset_selected(-1); // Reset to null selection when entering presets menu
        break;
        case SELECTION_SETTINGS_INFO:
            set_context(CTX_INFO);
            set_selection(SELECTION_INFO_BACK);
        break;
        case SELECTION_REVERB_ONOFF:
        case SELECTION_CHORUS_ONOFF:
        case SELECTION_ECHO_ONOFF:
        case SELECTION_FILTER_ONOFF:
        case SELECTION_DISTORTION_ONOFF:
        {
            amy_fx_t fx;
            if (selection == SELECTION_REVERB_ONOFF) fx = REVERB;
            else if (selection == SELECTION_CHORUS_ONOFF) fx = CHORUS;
            else if (selection == SELECTION_ECHO_ONOFF) fx = ECHO;
            else if (selection == SELECTION_FILTER_ONOFF) fx = FILTER;
            else fx = DISTORTION;
            set_fx(fx, !get_fx(fx));
            if (fx != DISTORTION) {
                update_fx(fx);
            }
            set_draw_pending(true);
        }
        break;
        case SELECTION_SETTINGS_LEFTHANDED:
           toggle_lefthanded();
           display_update_rotation(&display);
        break;
        case SELECTION_SETTINGS_PLAYING_MODE:
           toggle_playing_mode();
        break;
        case SELECTION_REVERB_RESET:
            reset_reverb_fx();
            update_fx(REVERB);
            set_draw_pending(true);
        break;
        case SELECTION_CHORUS_RESET:
            reset_chorus_fx();
            update_fx(CHORUS);
            set_draw_pending(true);
        break;
        case SELECTION_ECHO_RESET:
            reset_echo_fx();
            update_fx(ECHO);
            set_draw_pending(true);
        break;
        case SELECTION_FILTER_RESET:
            reset_filter_fx();
            update_fx(FILTER);
            set_draw_pending(true);
        break;
        case SELECTION_DISTORTION_RESET:
            reset_distortion_fx();
            set_draw_pending(true);
        break;
        case SELECTION_ADVANCED_RESET:
            reset_advanced_timing();
            set_draw_pending(true);
        break;
        case SELECTION_TUNING_RESET:
            reset_tuning();
            update_tuning();
            set_draw_pending(true);
        break;
        case SELECTION_PRESETS_SAVE:
            {
            // Save current state to selected preset (only if preset is valid)
            int8_t preset = get_preset_selected();
            if (preset >= 0 && preset < NUM_PRESETS) {
                save_preset(preset);
                set_preset_save_confirmed();
                set_draw_pending(true);
            }
            }
        break;
        case SELECTION_PRESETS_RESET:
            {
            // Reset selected preset to default values (only if preset is valid)
            int8_t preset = get_preset_selected();
            if (preset >= 0 && preset < NUM_PRESETS) {
                reset_preset(preset);
                set_draw_pending(true);
            }
            }
        break;
        case SELECTION_PRESETS_BACK:
            set_context(CTX_ADVANCED);
            set_selection(SELECTION_ADVANCED_PRESETS);
        break;
        case SELECTION_PATCH_NUM:
            set_patch(get_random_u8());
            update_patch();
        break;
        case SELECTION_INFO_BACK:
            set_context(CTX_SETTINGS);
            set_selection(SELECTION_SETTINGS_INFO);
            request_flash_write();
        break;
        case SELECTION_PATCH_BACK:
        case SELECTION_TUNING_BACK:
        case SELECTION_SETTINGS_BACK:
        case SELECTION_REVERB_BACK:
        case SELECTION_CHORUS_BACK:
        case SELECTION_ECHO_BACK:
        case SELECTION_FILTER_BACK:
        case SELECTION_DISTORTION_BACK:
        case SELECTION_ADVANCED_BACK:
            if (selection == SELECTION_ADVANCED_BACK) {
                set_context(CTX_SETTINGS);
                set_selection(SELECTION_SETTINGS_ADVANCED);
            } else {
                set_context(CTX_MAIN);
                set_selection(SELECTION_PATCH);
            }
            request_flash_write();
        break;
    }
}

void directional_switch_init() {
    gpio_init(DS_A);
    gpio_set_dir(DS_A, GPIO_IN);
    gpio_pull_up(DS_A);
    
    gpio_init(DS_B);
    gpio_set_dir(DS_B, GPIO_IN);
    gpio_pull_up(DS_B);
    
    gpio_init(DS_C);
    gpio_set_dir(DS_C, GPIO_IN);
    gpio_pull_up(DS_C);
    
    gpio_init(DS_D);
    gpio_set_dir(DS_D, GPIO_IN);
    gpio_pull_up(DS_D);
    
    gpio_init(DS_X);
    gpio_set_dir(DS_X, GPIO_IN);
    gpio_pull_up(DS_X);
}

void directional_switch_task() {
    static bool lefthanded = false;
    bool current_lefthanded = get_lefthanded();
    if (current_lefthanded != lefthanded) {
        lefthanded = current_lefthanded;
    }
    
    uint32_t now = time_us_32() / 1000; // ms
    
    for (int i = 0; i < 5; i++) {
        bool current_state = gpio_get(button_pins[i]);
        
        // Detect falling edge (button press - active low with pull-up)
        if (!current_state && last_button_state[i]) {
            button_press_time[i] = now;
            long_press_triggered[i] = false; // Reset long press flag on new press
            
            // Handle the button press
            // In left-handed mode, invert both axes: swap up/down and left/right
            if (lefthanded) {
                switch(button_pins[i]) {
                    case DS_A:
                        ds_down(); // Inverted: up becomes down
                        break;
                    case DS_B:
                        ds_right(); // Inverted: left becomes right
                        break;
                    case DS_C:
                        ds_up(); // Inverted: down becomes up
                        break;
                    case DS_D:
                        ds_left(); // Inverted: right becomes left
                        break;
                    case DS_X:
                        ds_center();
                        break;
                }
            } else {
                switch(button_pins[i]) {
                    case DS_A:
                        ds_up();
                        break;
                    case DS_B:
                        ds_left();
                        break;
                    case DS_C:
                        ds_down();
                        break;
                    case DS_D:
                        ds_right();
                        break;
                    case DS_X:
                        ds_center();
                        break;
                }
            }
            set_draw_pending(true);
        }
        // Check for long press (button still held down)
        else if (!current_state && !last_button_state[i]) {
            if (!long_press_triggered[i] && (now - button_press_time[i] > LONG_PRESS_THRESHOLD)) {
                context_t context = get_context();
                selection_t selection = get_selection();
                
                if (button_pins[i] == DS_X) {
                    if (context == CTX_MAIN) {
                        // Long press center button opens effect screens when effect is selected
                        if (selection == SELECTION_REVERB) {
                            set_context(CTX_REVERB);
                            set_selection(SELECTION_REVERB_ONOFF);
                        } else if (selection == SELECTION_CHORUS) {
                            set_context(CTX_CHORUS);
                            set_selection(SELECTION_CHORUS_ONOFF);
                        } else if (selection == SELECTION_ECHO) {
                            set_context(CTX_ECHO);
                            set_selection(SELECTION_ECHO_ONOFF);
                        } else if (selection == SELECTION_FILTER) {
                            set_context(CTX_FILTER);
                            set_selection(SELECTION_FILTER_ONOFF);
                        } else if (selection == SELECTION_DISTORTION) {
                            set_context(CTX_DISTORTION);
                            set_selection(SELECTION_DISTORTION_ONOFF);
                        }
                    }
                    set_draw_pending(true);
                    long_press_triggered[i] = true; // Mark as triggered to prevent repeated toggling
                }
            }
        }
        // Detect rising edge (button release)
        else if (current_state && !last_button_state[i]) {
            // Reset long press flag when button is released
            long_press_triggered[i] = false;
        }
        
        last_button_state[i] = current_state;
    }
}
