#include "pico/stdlib.h"
#include "state_data.h"
#include "config.h"
#include "flash.h"
#include <stdlib.h>

// Declare the static state_data instance
static state_data_t state_data;

state_data_t* get_state_data(void) {
    return &state_data;
}

/* Patch */

uint8_t get_patch() {
    return state_data.patch;
}

void set_patch(uint8_t value) {
    state_data.patch = value;
    set_dirty(true);
}

void set_patch_up() {
    uint8_t patch = get_patch();
    patch++;
    set_patch(patch);
}

void set_patch_down() {
    uint8_t patch = get_patch();
    patch--;
    set_patch(patch);
}

/* Context */
uint8_t get_context() {
    return state_data.context;
}

void set_context(context_t value) {
    state_data.context = value;
}

/* Selection */
uint8_t get_selection() {
    return state_data.selection;
}

void set_selection(selection_t value) {
    state_data.selection = value;
}

void set_selection_up() {
    selection_t selection = get_selection();
    context_t context = get_context();
    
    // Context-aware navigation with wrapping
    switch(context) {
        case CTX_PATCH: {
            selection_t valid[] = {SELECTION_PATCH_NUM, SELECTION_PATCH_BACK};
            uint8_t count = 2;
            for(uint8_t i = 0; i < count; i++) {
                if(valid[i] == selection) {
                    selection = valid[(i - 1 + count) % count];
                    break;
                }
            }
            break;
        }
        case CTX_TUNING: {
            selection_t valid[] = {SELECTION_STRING_0, SELECTION_STRING_1, SELECTION_STRING_2, SELECTION_STRING_3, SELECTION_CAPO, SELECTION_TUNING_RESET, SELECTION_TUNING_BACK};
            uint8_t count = 7;
            for(uint8_t i = 0; i < count; i++) {
                if(valid[i] == selection) {
                    selection = valid[(i - 1 + count) % count];
                    break;
                }
            }
            break;
        }
        case CTX_SETTINGS: {
            selection_t valid[] = {SELECTION_SETTINGS_PLAYING_MODE, SELECTION_SETTINGS_LEFTHANDED, SELECTION_SETTINGS_VOLUME, SELECTION_SETTINGS_CONTRAST, SELECTION_SETTINGS_ADVANCED, SELECTION_SETTINGS_INFO, SELECTION_SETTINGS_BACK};
            uint8_t count = 7;
            for(uint8_t i = 0; i < count; i++) {
                if(valid[i] == selection) {
                    selection = valid[(i - 1 + count) % count];
                    break;
                }
            }
            break;
        }
        case CTX_ADVANCED: {
            selection_t valid[] = {SELECTION_ADVANCED_SNAPSHOT_WINDOW, SELECTION_ADVANCED_STALE_TIMEOUT, SELECTION_ADVANCED_VERY_RECENT, SELECTION_ADVANCED_POST_STRUM, SELECTION_ADVANCED_RELEASE_DELAY, SELECTION_ADVANCED_PRESETS, SELECTION_ADVANCED_RESET, SELECTION_ADVANCED_BACK};
            uint8_t count = 8;
            for(uint8_t i = 0; i < count; i++) {
                if(valid[i] == selection) {
                    selection = valid[(i - 1 + count) % count];
                    break;
                }
            }
            break;
        }
        case CTX_INFO: {
            selection_t valid[] = {SELECTION_INFO_BACK};
            uint8_t count = 1;
            // Only one selection, so no navigation needed
            break;
        }
        case CTX_PRESETS: {
            selection_t valid[] = {SELECTION_PRESETS_SAVE, SELECTION_PRESETS_LOAD, SELECTION_PRESETS_RESET, SELECTION_PRESETS_BACK};
            uint8_t count = 4;
            for(uint8_t i = 0; i < count; i++) {
                if(valid[i] == selection) {
                    selection = valid[(i - 1 + count) % count];
                    break;
                }
            }
            break;
        }
        case CTX_REVERB: {
            selection_t valid[] = {SELECTION_REVERB_ONOFF, SELECTION_REVERB_LIVENESS, SELECTION_REVERB_DAMPING, SELECTION_REVERB_XOVER, SELECTION_REVERB_RESET, SELECTION_REVERB_BACK};
            uint8_t count = 6;
            for(uint8_t i = 0; i < count; i++) {
                if(valid[i] == selection) {
                    selection = valid[(i - 1 + count) % count];
                    break;
                }
            }
            break;
        }
        case CTX_CHORUS: {
            selection_t valid[] = {SELECTION_CHORUS_ONOFF, SELECTION_CHORUS_MAX_DELAY, SELECTION_CHORUS_LFO_FREQ, SELECTION_CHORUS_DEPTH, SELECTION_CHORUS_RESET, SELECTION_CHORUS_BACK};
            uint8_t count = 6;
            for(uint8_t i = 0; i < count; i++) {
                if(valid[i] == selection) {
                    selection = valid[(i - 1 + count) % count];
                    break;
                }
            }
            break;
        }
        case CTX_ECHO: {
            selection_t valid[] = {SELECTION_ECHO_ONOFF, SELECTION_ECHO_DELAY, SELECTION_ECHO_FEEDBACK, SELECTION_ECHO_FILTER, SELECTION_ECHO_RESET, SELECTION_ECHO_BACK};
            uint8_t count = 6;
            for(uint8_t i = 0; i < count; i++) {
                if(valid[i] == selection) {
                    selection = valid[(i - 1 + count) % count];
                    break;
                }
            }
            break;
        }
        case CTX_FILTER: {
            selection_t valid[] = {SELECTION_FILTER_ONOFF, SELECTION_FILTER_FREQ, SELECTION_FILTER_RESONANCE, SELECTION_FILTER_RESET, SELECTION_FILTER_BACK};
            uint8_t count = 5;
            for(uint8_t i = 0; i < count; i++) {
                if(valid[i] == selection) {
                    selection = valid[(i - 1 + count) % count];
                    break;
                }
            }
            break;
        }
        case CTX_DISTORTION: {
            selection_t valid[] = {SELECTION_DISTORTION_ONOFF, SELECTION_DISTORTION_LEVEL, SELECTION_DISTORTION_GAIN, SELECTION_DISTORTION_RESET, SELECTION_DISTORTION_BACK};
            uint8_t count = 5;
            for(uint8_t i = 0; i < count; i++) {
                if(valid[i] == selection) {
                    selection = valid[(i - 1 + count) % count];
                    break;
                }
            }
            break;
        }
        case CTX_MAIN: {
            selection_t valid[] = {SELECTION_PATCH, SELECTION_REVERB, SELECTION_FILTER, SELECTION_CHORUS, SELECTION_ECHO, SELECTION_DISTORTION, SELECTION_TUNING, SELECTION_SETTINGS};
            uint8_t count = 8;
            for(uint8_t i = 0; i < count; i++) {
                if(valid[i] == selection) {
                    selection = valid[(i - 1 + count) % count];
                    break;
                }
            }
            break;
        }
        default:
            // For other contexts, no navigation
            break;
    }
    
    set_selection(selection);
}

void set_selection_down() {
    selection_t selection = get_selection();
    context_t context = get_context();
    
    switch(context) {
        case CTX_PATCH: {
            selection_t valid[] = {SELECTION_PATCH_NUM, SELECTION_PATCH_BACK};
            uint8_t count = 2;
            for(uint8_t i = 0; i < count; i++) {
                if(valid[i] == selection) {
                    selection = valid[(i + 1) % count];
                    break;
                }
            }
            break;
        }
        case CTX_TUNING: {
            selection_t valid[] = {SELECTION_STRING_0, SELECTION_STRING_1, SELECTION_STRING_2, SELECTION_STRING_3, SELECTION_CAPO, SELECTION_TUNING_RESET, SELECTION_TUNING_BACK};
            uint8_t count = 7;
            for(uint8_t i = 0; i < count; i++) {
                if(valid[i] == selection) {
                    selection = valid[(i + 1) % count];
                    break;
                }
            }
            break;
        }
        case CTX_SETTINGS: {
            selection_t valid[] = {SELECTION_SETTINGS_PLAYING_MODE, SELECTION_SETTINGS_LEFTHANDED, SELECTION_SETTINGS_VOLUME, SELECTION_SETTINGS_CONTRAST, SELECTION_SETTINGS_ADVANCED, SELECTION_SETTINGS_INFO, SELECTION_SETTINGS_BACK};
            uint8_t count = 7;
            for(uint8_t i = 0; i < count; i++) {
                if(valid[i] == selection) {
                    selection = valid[(i + 1) % count];
                    break;
                }
            }
            break;
        }
        case CTX_ADVANCED: {
            selection_t valid[] = {SELECTION_ADVANCED_SNAPSHOT_WINDOW, SELECTION_ADVANCED_STALE_TIMEOUT, SELECTION_ADVANCED_VERY_RECENT, SELECTION_ADVANCED_POST_STRUM, SELECTION_ADVANCED_RELEASE_DELAY, SELECTION_ADVANCED_PRESETS, SELECTION_ADVANCED_RESET, SELECTION_ADVANCED_BACK};
            uint8_t count = 8;
            for(uint8_t i = 0; i < count; i++) {
                if(valid[i] == selection) {
                    selection = valid[(i + 1) % count];
                    break;
                }
            }
            break;
        }
        case CTX_INFO: {
            selection_t valid[] = {SELECTION_INFO_BACK};
            uint8_t count = 1;
            break;
        }
        case CTX_PRESETS: {
            selection_t valid[] = {SELECTION_PRESETS_SAVE, SELECTION_PRESETS_LOAD, SELECTION_PRESETS_RESET, SELECTION_PRESETS_BACK};
            uint8_t count = 4;
            for(uint8_t i = 0; i < count; i++) {
                if(valid[i] == selection) {
                    selection = valid[(i + 1) % count];
                    break;
                }
            }
            break;
        }
        case CTX_REVERB: {
            selection_t valid[] = {SELECTION_REVERB_ONOFF, SELECTION_REVERB_LIVENESS, SELECTION_REVERB_DAMPING, SELECTION_REVERB_XOVER, SELECTION_REVERB_RESET, SELECTION_REVERB_BACK};
            uint8_t count = 6;
            for(uint8_t i = 0; i < count; i++) {
                if(valid[i] == selection) {
                    selection = valid[(i + 1) % count];
                    break;
                }
            }
            break;
        }
        case CTX_CHORUS: {
            selection_t valid[] = {SELECTION_CHORUS_ONOFF, SELECTION_CHORUS_MAX_DELAY, SELECTION_CHORUS_LFO_FREQ, SELECTION_CHORUS_DEPTH, SELECTION_CHORUS_RESET, SELECTION_CHORUS_BACK};
            uint8_t count = 6;
            for(uint8_t i = 0; i < count; i++) {
                if(valid[i] == selection) {
                    selection = valid[(i + 1) % count];
                    break;
                }
            }
            break;
        }
        case CTX_ECHO: {
            selection_t valid[] = {SELECTION_ECHO_ONOFF, SELECTION_ECHO_DELAY, SELECTION_ECHO_FEEDBACK, SELECTION_ECHO_FILTER, SELECTION_ECHO_RESET, SELECTION_ECHO_BACK};
            uint8_t count = 6;
            for(uint8_t i = 0; i < count; i++) {
                if(valid[i] == selection) {
                    selection = valid[(i + 1) % count];
                    break;
                }
            }
            break;
        }
        case CTX_FILTER: {
            selection_t valid[] = {SELECTION_FILTER_ONOFF, SELECTION_FILTER_FREQ, SELECTION_FILTER_RESONANCE, SELECTION_FILTER_RESET, SELECTION_FILTER_BACK};
            uint8_t count = 5;
            for(uint8_t i = 0; i < count; i++) {
                if(valid[i] == selection) {
                    selection = valid[(i + 1) % count];
                    break;
                }
            }
            break;
        }
        case CTX_DISTORTION: {
            selection_t valid[] = {SELECTION_DISTORTION_ONOFF, SELECTION_DISTORTION_LEVEL, SELECTION_DISTORTION_GAIN, SELECTION_DISTORTION_RESET, SELECTION_DISTORTION_BACK};
            uint8_t count = 5;
            for(uint8_t i = 0; i < count; i++) {
                if(valid[i] == selection) {
                    selection = valid[(i + 1) % count];
                    break;
                }
            }
            break;
        }
        case CTX_MAIN: {
            selection_t valid[] = {SELECTION_PATCH, SELECTION_REVERB, SELECTION_FILTER, SELECTION_CHORUS, SELECTION_ECHO, SELECTION_DISTORTION, SELECTION_TUNING, SELECTION_SETTINGS};
            uint8_t count = 8;
            for(uint8_t i = 0; i < count; i++) {
                if(valid[i] == selection) {
                    selection = valid[(i + 1) % count];
                    break;
                }
            }
            break;
        }
        default:
            break;
    }
    
    set_selection(selection);
}

/* FX */

bool get_fx(amy_fx_t fx){
    switch(fx){
        case REVERB:
            return state_data.fx_reverb;
        break;
        case FILTER:
            return state_data.fx_filter;
        break;
        case CHORUS:
            return state_data.fx_chorus;
        break;
        case ECHO:
            return state_data.fx_echo;
        break;
        case DISTORTION:
            return state_data.fx_distortion;
        break;
        default:
            return false;
    }
}

void set_fx(amy_fx_t fx, bool value){
    switch(fx){
        case REVERB:
            state_data.fx_reverb = value;
        break;
        case FILTER:
            state_data.fx_filter = value;
        break;
        case CHORUS:
            state_data.fx_chorus = value;
        break;
        case ECHO:
            state_data.fx_echo = value;
        break;
        case DISTORTION:
            state_data.fx_distortion = value;
            // Update global distortion enabled state
            extern void global_distortion_set_enabled(bool enabled);
            global_distortion_set_enabled(value);
            // Configure distortion parameters when enabling (ensures parameters are set even after boot)
            if (value) {
                extern void config_global_distortion(float level, float gain);
                config_global_distortion(get_distortion_level(), get_distortion_gain());
            }
        break;
    }
    set_dirty(true);
}

/* Volume */

uint8_t get_volume() {
    return state_data.volume;
}

void set_volume(uint8_t value) {
    // Clamp volume to 0-8 range (UI uses 0-8, gets converted to AMY's 0-11.0 range)
    if (value > 8) {
        value = 8;
    }
    state_data.volume = value;
    set_dirty(true);
}

void set_volume_up() {
    uint8_t volume = get_volume();
    if (volume < 8) {
        set_volume(volume + 1);
    }
}

void set_volume_down() {
    uint8_t volume = get_volume();
    if (volume > 0) {
        set_volume(volume - 1);
    }
}

/* Contrast */
// 0 - Minimum
// 1 - Medium
// 2 - Maximum
// 3 - Automatic dimming

uint8_t get_contrast() {
    return state_data.contrast;
}

void set_contrast(uint8_t value) {
    state_data.contrast = value;
    set_dirty(true);
}

void set_contrast_up() {
    uint8_t contrast = get_contrast();
    if(contrast < 3) {
        set_contrast(contrast + 1);
    }
}

void set_contrast_down() {
    uint8_t contrast = get_contrast();
    if(contrast > 0) {
        set_contrast(contrast - 1);
    }
}

/* String pitch */

uint8_t get_string_pitch(uint8_t string) {
    if (string >= NUM_STRINGS) {
        return 0;  // Invalid string index
    }
    return state_data.string_pitch[string];
}

void set_string_pitch(uint8_t string, uint8_t value) {
    if (string >= NUM_STRINGS) {
        return;  // Invalid string index
    }
    state_data.string_pitch[string] = value;
    set_dirty(true);
}

void set_string_pitch_up(uint8_t string) {
    uint8_t pitch = get_string_pitch(string);
    if (pitch < MIDI_NOTE_MAX) {
        set_string_pitch(string, pitch + 1);
    }
}

void set_string_pitch_down(uint8_t string) {
    uint8_t pitch = get_string_pitch(string);
    if (pitch > 0) {
        set_string_pitch(string, pitch - 1);
    }
}

/* Capo */

int16_t get_capo() {
    return state_data.capo;
}

void set_capo(int16_t value) {
    state_data.capo = value;
    set_dirty(true);
}

void set_capo_up() {
    int16_t capo = get_capo();
    if (capo < 24) {
        set_capo(capo + 1);
    }
}

void set_capo_down() {
    int16_t capo = get_capo();
    if (capo > -24) {
        set_capo(capo - 1);
    }
}

void reset_tuning() {
    set_string_pitch(0, DEFAULT_STRING_PITCH_0);
    set_string_pitch(1, DEFAULT_STRING_PITCH_1);
    set_string_pitch(2, DEFAULT_STRING_PITCH_2);
    set_string_pitch(3, DEFAULT_STRING_PITCH_3);
    set_capo(0);
}

/* Playing mode */

bool get_playing_mode() {
    return state_data.playing_mode;
}

void set_playing_mode(bool value) {
    state_data.playing_mode = value;
    set_dirty(true);
}

void toggle_playing_mode() {
    state_data.playing_mode = ! state_data.playing_mode;
    set_dirty(true);
}

/* Lefthanded */

bool get_lefthanded() {
    return state_data.lefthanded;
}

void set_lefthanded(bool value) {
    state_data.lefthanded = value;
    set_dirty(true);
}

void toggle_lefthanded() {
    state_data.lefthanded = ! state_data.lefthanded;
    set_dirty(true);
}

/* Dirty */

bool get_dirty() {
    return state_data.dirty;
}

void set_dirty(bool value) {
    state_data.dirty = value;
    // Automatically schedule flash write when settings become dirty
    // so settings are saved even if user doesn't navigate back from screens.
    if (value) {
        request_flash_write();
    }
}

/* Low batt */

bool get_low_batt() {
    return state_data.low_batt;
}

void set_low_batt(bool value) {
    state_data.low_batt = value;
}

/* Preset selected */

int8_t get_preset_selected() {
    return state_data.preset_selected;
}

void set_preset_selected(int8_t value) {
    if (value < -1) value = -1;
    if (value > 3) value = 3;
    state_data.preset_selected = value;
}

/* Utilities and railroad stations */

uint8_t get_random_u8() {
    static bool seeded;
    if(!seeded){
        srand(time_us_64());
        seeded = true;
    }
    return (uint8_t)(rand() % 256);
}

/* Reverb parameters */

float get_reverb_liveness() {
    return state_data.reverb_liveness;
}

void set_reverb_liveness(float value) {
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;
    state_data.reverb_liveness = value;
    set_dirty(true);
}

void set_reverb_liveness_up() {
    float val = get_reverb_liveness();
    set_reverb_liveness(val + 0.05f);
}

void set_reverb_liveness_down() {
    float val = get_reverb_liveness();
    set_reverb_liveness(val - 0.05f);
}

float get_reverb_damping() {
    return state_data.reverb_damping;
}

void set_reverb_damping(float value) {
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;
    state_data.reverb_damping = value;
    set_dirty(true);
}

void set_reverb_damping_up() {
    float val = get_reverb_damping();
    set_reverb_damping(val + 0.05f);
}

void set_reverb_damping_down() {
    float val = get_reverb_damping();
    set_reverb_damping(val - 0.05f);
}

float get_reverb_xover_hz() {
    return state_data.reverb_xover_hz;
}

void set_reverb_xover_hz(float value) {
    if (value < 100.0f) value = 100.0f;
    if (value > 10000.0f) value = 10000.0f;
    state_data.reverb_xover_hz = value;
    set_dirty(true);
}

void set_reverb_xover_hz_up() {
    float val = get_reverb_xover_hz();
    set_reverb_xover_hz(val + 100.0f);
}

void set_reverb_xover_hz_down() {
    float val = get_reverb_xover_hz();
    set_reverb_xover_hz(val - 100.0f);
}

void reset_reverb_fx() {
    set_reverb_liveness(DIAPASONIX_REVERB_DEFAULT_LIVENESS);
    set_reverb_damping(DIAPASONIX_REVERB_DEFAULT_DAMPING);
    set_reverb_xover_hz(DIAPASONIX_REVERB_DEFAULT_XOVER_HZ);
}

/* Chorus parameters */

int get_chorus_max_delay() {
    return state_data.chorus_max_delay;
}

void set_chorus_max_delay(int value) {
    if (value < 10) value = 10;
    if (value > 640) value = 640;
    state_data.chorus_max_delay = value;
    set_dirty(true);
}

void set_chorus_max_delay_up() {
    int val = get_chorus_max_delay();
    set_chorus_max_delay(val + 10);
}

void set_chorus_max_delay_down() {
    int val = get_chorus_max_delay();
    set_chorus_max_delay(val - 10);
}

float get_chorus_lfo_freq() {
    return state_data.chorus_lfo_freq;
}

void set_chorus_lfo_freq(float value) {
    if (value < 0.1f) value = 0.1f;
    if (value > 10.0f) value = 10.0f;
    state_data.chorus_lfo_freq = value;
    set_dirty(true);
}

void set_chorus_lfo_freq_up() {
    float val = get_chorus_lfo_freq();
    set_chorus_lfo_freq(val + 0.1f);
}

void set_chorus_lfo_freq_down() {
    float val = get_chorus_lfo_freq();
    set_chorus_lfo_freq(val - 0.1f);
}

float get_chorus_depth() {
    return state_data.chorus_depth;
}

void set_chorus_depth(float value) {
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;
    state_data.chorus_depth = value;
    set_dirty(true);
}

void set_chorus_depth_up() {
    float val = get_chorus_depth();
    set_chorus_depth(val + 0.05f);
}

void set_chorus_depth_down() {
    float val = get_chorus_depth();
    set_chorus_depth(val - 0.05f);
}

void reset_chorus_fx() {
    set_chorus_max_delay(DIAPASONIX_CHORUS_DEFAULT_MAX_DELAY);
    set_chorus_lfo_freq(DIAPASONIX_CHORUS_DEFAULT_LFO_FREQ);
    set_chorus_depth(DIAPASONIX_CHORUS_DEFAULT_MOD_DEPTH);
}

/* Echo parameters */

float get_echo_delay_ms() {
    return state_data.echo_delay_ms;
}

void set_echo_delay_ms(float value) {
    if (value < 10.0f) value = 10.0f;
    if (value > DIAPASONIX_ECHO_DEFAULT_MAX_DELAY_MS) value = DIAPASONIX_ECHO_DEFAULT_MAX_DELAY_MS;  // Limited by RP2350 RAM
    state_data.echo_delay_ms = value;
    set_dirty(true);
}

void set_echo_delay_ms_up() {
    float val = get_echo_delay_ms();
    set_echo_delay_ms(val + 10.0f);
}

void set_echo_delay_ms_down() {
    float val = get_echo_delay_ms();
    set_echo_delay_ms(val - 10.0f);
}

float get_echo_feedback() {
    return state_data.echo_feedback;
}

void set_echo_feedback(float value) {
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;
    state_data.echo_feedback = value;
    set_dirty(true);
}

void set_echo_feedback_up() {
    float val = get_echo_feedback();
    set_echo_feedback(val + 0.05f);
}

void set_echo_feedback_down() {
    float val = get_echo_feedback();
    set_echo_feedback(val - 0.05f);
}

float get_echo_filter_coef() {
    return state_data.echo_filter_coef;
}

void set_echo_filter_coef(float value) {
    if (value < -0.99f) value = -0.99f;
    if (value > 0.99f) value = 0.99f;
    state_data.echo_filter_coef = value;
    set_dirty(true);
}

void set_echo_filter_coef_up() {
    float val = get_echo_filter_coef();
    set_echo_filter_coef(val + 0.05f);
}

void set_echo_filter_coef_down() {
    float val = get_echo_filter_coef();
    set_echo_filter_coef(val - 0.05f);
}

void reset_echo_fx() {
    set_echo_delay_ms(DIAPASONIX_ECHO_DEFAULT_DELAY_MS);
    set_echo_feedback((float)DIAPASONIX_ECHO_DEFAULT_FEEDBACK);
    set_echo_filter_coef((float)DIAPASONIX_ECHO_DEFAULT_FILTER_COEF);
}

/* Filter parameters */

float get_filter_freq_hz() {
    return state_data.filter_freq_hz;
}

void set_filter_freq_hz(float value) {
    // Clamp frequency to reasonable range: 20 Hz to 20 kHz
    if (value < 20.0f) value = 20.0f;
    if (value > 20000.0f) value = 20000.0f;
    state_data.filter_freq_hz = value;
    set_dirty(true);
}

void set_filter_freq_hz_up() {
    float val = get_filter_freq_hz();
    // Exponential scaling for frequency adjustment
    val *= 1.1f;
    set_filter_freq_hz(val);
}

void set_filter_freq_hz_down() {
    float val = get_filter_freq_hz();
    // Exponential scaling for frequency adjustment
    val /= 1.1f;
    set_filter_freq_hz(val);
}

float get_filter_resonance() {
    return state_data.filter_resonance;
}

void set_filter_resonance(float value) {
    // Clamp resonance to valid range: 0.5 to 16.0
    if (value < 0.5f) value = 0.5f;
    if (value > 16.0f) value = 16.0f;
    state_data.filter_resonance = value;
    set_dirty(true);
}

void set_filter_resonance_up() {
    float val = get_filter_resonance();
    set_filter_resonance(val + 0.1f);
}

void set_filter_resonance_down() {
    float val = get_filter_resonance();
    set_filter_resonance(val - 0.1f);
}

void reset_filter_fx() {
    set_filter_freq_hz(DIAPASONIX_FILTER_DEFAULT_FREQ_HZ);
    set_filter_resonance(DIAPASONIX_FILTER_DEFAULT_RESONANCE);
}

/* Distortion parameters */

float get_distortion_level() {
    return state_data.distortion_level;
}

void set_distortion_level(float value) {
    // Clamp level to 0.0 to 1.0
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;
    state_data.distortion_level = value;
    set_dirty(true);
    // Update global distortion configuration
    extern void config_global_distortion(float level, float gain);
    config_global_distortion(value, get_distortion_gain());
}

void set_distortion_level_up() {
    float val = get_distortion_level();
    set_distortion_level(val + 0.05f);
}

void set_distortion_level_down() {
    float val = get_distortion_level();
    set_distortion_level(val - 0.05f);
}

float get_distortion_gain() {
    return state_data.distortion_gain;
}

void set_distortion_gain(float value) {
    // Clamp gain to 10.0 to 20.0 (internal range, displayed as 1.0 to 2.0 in UI)
    if (value < 10.0f) value = 10.0f;
    if (value > 20.0f) value = 20.0f;
    state_data.distortion_gain = value;
    set_dirty(true);
    // Update global distortion configuration
    extern void config_global_distortion(float level, float gain);
    config_global_distortion(get_distortion_level(), value);
}

void set_distortion_gain_up() {
    float val = get_distortion_gain();
    set_distortion_gain(val + 1.0f);  // Increment by 1.0 internally (0.1 in UI)
}

void set_distortion_gain_down() {
    float val = get_distortion_gain();
    set_distortion_gain(val - 1.0f);  // Decrement by 1.0 internally (0.1 in UI)
}

void reset_distortion_fx() {
    set_distortion_level(DIAPASONIX_DISTORTION_DEFAULT_LEVEL);
    set_distortion_gain(DIAPASONIX_DISTORTION_DEFAULT_GAIN);
}

/* Advanced timing parameters */

uint32_t get_state_snapshot_window_ms() {
    return state_data.state_snapshot_window_ms;
}

void set_state_snapshot_window_ms(uint32_t value) {
    if (value < 10) value = 10;
    if (value > 1000) value = 1000;
    state_data.state_snapshot_window_ms = value;
    set_dirty(true);
}

void set_state_snapshot_window_ms_up() {
    uint32_t val = get_state_snapshot_window_ms();
    set_state_snapshot_window_ms(val + 10);
}

void set_state_snapshot_window_ms_down() {
    uint32_t val = get_state_snapshot_window_ms();
    if (val > 10) {
        set_state_snapshot_window_ms(val - 10);
    }
}

uint32_t get_fret_stale_timeout_ms() {
    return state_data.fret_stale_timeout_ms;
}

void set_fret_stale_timeout_ms(uint32_t value) {
    if (value < 100) value = 100;
    if (value > 30000) value = 30000;
    state_data.fret_stale_timeout_ms = value;
    set_dirty(true);
}

void set_fret_stale_timeout_ms_up() {
    uint32_t val = get_fret_stale_timeout_ms();
    set_fret_stale_timeout_ms(val + 100);
}

void set_fret_stale_timeout_ms_down() {
    uint32_t val = get_fret_stale_timeout_ms();
    if (val > 100) {
        set_fret_stale_timeout_ms(val - 100);
    }
}

uint32_t get_fret_very_recent_threshold_ms() {
    return state_data.fret_very_recent_threshold_ms;
}

void set_fret_very_recent_threshold_ms(uint32_t value) {
    if (value < 10) value = 10;
    if (value > 500) value = 500;
    state_data.fret_very_recent_threshold_ms = value;
    set_dirty(true);
}

void set_fret_very_recent_threshold_ms_up() {
    uint32_t val = get_fret_very_recent_threshold_ms();
    set_fret_very_recent_threshold_ms(val + 10);
}

void set_fret_very_recent_threshold_ms_down() {
    uint32_t val = get_fret_very_recent_threshold_ms();
    if (val > 10) {
        set_fret_very_recent_threshold_ms(val - 10);
    }
}

uint32_t get_fret_post_strum_threshold_ms() {
    return state_data.fret_post_strum_threshold_ms;
}

void set_fret_post_strum_threshold_ms(uint32_t value) {
    if (value < 10) value = 10;
    if (value > 500) value = 500;
    state_data.fret_post_strum_threshold_ms = value;
    set_dirty(true);
}

void set_fret_post_strum_threshold_ms_up() {
    uint32_t val = get_fret_post_strum_threshold_ms();
    set_fret_post_strum_threshold_ms(val + 10);
}

void set_fret_post_strum_threshold_ms_down() {
    uint32_t val = get_fret_post_strum_threshold_ms();
    if (val > 10) {
        set_fret_post_strum_threshold_ms(val - 10);
    }
}

uint32_t get_fret_release_delay_ms() {
    return state_data.fret_release_delay_ms;
}

void set_fret_release_delay_ms(uint32_t value) {
    if (value < 0) value = 0;
    if (value > 500) value = 500;
    state_data.fret_release_delay_ms = value;
    set_dirty(true);
}

void set_fret_release_delay_ms_up() {
    uint32_t val = get_fret_release_delay_ms();
    set_fret_release_delay_ms(val + 10);
}

void set_fret_release_delay_ms_down() {
    uint32_t val = get_fret_release_delay_ms();
    if (val > 0) {
        set_fret_release_delay_ms(val - 10);
    }
}

void reset_advanced_timing() {
    set_state_snapshot_window_ms(STATE_SNAPSHOT_WINDOW_MS);
    set_fret_stale_timeout_ms(FRET_STALE_TIMEOUT_MS);
    set_fret_very_recent_threshold_ms(FRET_VERY_RECENT_THRESHOLD_MS);
    set_fret_post_strum_threshold_ms(FRET_POST_STRUM_THRESHOLD_MS);
    set_fret_release_delay_ms(FRET_RELEASE_DELAY_MS);
}
