#ifndef STATE_DATA_H_
#define STATE_DATA_H_

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum context {
    CTX_INIT,
    CTX_INFO,
    CTX_MAIN,
    CTX_PATCH,
    CTX_REVERB,
    CTX_FILTER,
    CTX_CHORUS,
    CTX_ECHO,
    CTX_DISTORTION,
    CTX_TUNING,
    CTX_SETTINGS,
    CTX_ADVANCED,
    CTX_PRESETS,
    // CTX_LOOPER, // I'll add this when I grow older
} context_t;

typedef enum selection {
    /* Main screen */
    SELECTION_PATCH,
    SELECTION_REVERB,
    SELECTION_FILTER,
    SELECTION_CHORUS,
    SELECTION_ECHO,
    SELECTION_DISTORTION,
    SELECTION_TUNING,
    SELECTION_SETTINGS,

    /* Patch screen */
    SELECTION_PATCH_NUM,
    SELECTION_PATCH_BACK,

    /* Tuning screen */
    SELECTION_STRING_0,
    SELECTION_STRING_1,
    SELECTION_STRING_2,
    SELECTION_STRING_3,
    SELECTION_CAPO,
    SELECTION_TUNING_RESET,
    SELECTION_TUNING_BACK,

    /* Settings screen */
    SELECTION_SETTINGS_PLAYING_MODE,
    SELECTION_SETTINGS_LEFTHANDED,
    SELECTION_SETTINGS_VOLUME,
    SELECTION_SETTINGS_CONTRAST,
    SELECTION_SETTINGS_ADVANCED,
    SELECTION_SETTINGS_INFO,
    SELECTION_SETTINGS_BACK,

    /* Advanced screen */
    SELECTION_ADVANCED_SNAPSHOT_WINDOW,
    SELECTION_ADVANCED_STALE_TIMEOUT,
    SELECTION_ADVANCED_VERY_RECENT,
    SELECTION_ADVANCED_POST_STRUM,
    SELECTION_ADVANCED_RELEASE_DELAY,
    SELECTION_ADVANCED_PRESETS,
    SELECTION_ADVANCED_RESET,
    SELECTION_ADVANCED_BACK,

    /* Presets screen */
    SELECTION_PRESETS_SAVE,
    SELECTION_PRESETS_LOAD,
    SELECTION_PRESETS_RESET,
    SELECTION_PRESETS_BACK,

    /* Info screen */
    SELECTION_INFO_BACK,

    /* Reverb screen */
    SELECTION_REVERB_ONOFF,
    SELECTION_REVERB_LIVENESS,
    SELECTION_REVERB_DAMPING,
    SELECTION_REVERB_XOVER,
    SELECTION_REVERB_RESET,
    SELECTION_REVERB_BACK,

    /* Chorus screen */
    SELECTION_CHORUS_ONOFF,
    SELECTION_CHORUS_MAX_DELAY,
    SELECTION_CHORUS_LFO_FREQ,
    SELECTION_CHORUS_DEPTH,
    SELECTION_CHORUS_RESET,
    SELECTION_CHORUS_BACK,

    /* Echo screen */
    SELECTION_ECHO_ONOFF,
    SELECTION_ECHO_DELAY,
    SELECTION_ECHO_FEEDBACK,
    SELECTION_ECHO_FILTER,
    SELECTION_ECHO_RESET,
    SELECTION_ECHO_BACK,

    /* Filter screen */
    SELECTION_FILTER_ONOFF,
    SELECTION_FILTER_FREQ,
    SELECTION_FILTER_RESONANCE,
    SELECTION_FILTER_RESET,
    SELECTION_FILTER_BACK,

    /* Distortion screen */
    SELECTION_DISTORTION_ONOFF,
    SELECTION_DISTORTION_LEVEL,
    SELECTION_DISTORTION_GAIN,
    SELECTION_DISTORTION_RESET,
    SELECTION_DISTORTION_BACK,

    SELECTION_LAST,
} selection_t;

typedef struct state_data {
    uint8_t patch;
    context_t context;              // The directional switch affects different
                                    // parameters according to the current context
    selection_t selection;
    
    bool fx_reverb;
    bool fx_filter;
    bool fx_chorus;
    bool fx_echo;
    bool fx_distortion;

    // Effect parameters
    float reverb_liveness;
    float reverb_damping;
    float reverb_xover_hz;
    
    int chorus_max_delay;
    float chorus_lfo_freq;
    float chorus_depth;
    
    float echo_delay_ms;
    float echo_feedback;
    float echo_filter_coef;
    
    float filter_freq_hz;      // Filter cutoff frequency in Hz
    float filter_resonance;    // Filter Q factor (resonance)
    
    float distortion_level;    // Distortion amount (0.0 to 1.0)
    float distortion_gain;     // Distortion drive/gain (10.0 to 20.0 internally, displayed as 1.0 to 2.0)

    uint8_t volume;
    uint8_t contrast;          // Value to control the SSD1306 display brightness (aka "contrast")

    uint8_t string_pitch[NUM_STRINGS];
    int16_t capo;

    bool lefthanded;
    bool playing_mode;              // When true: strum mode (requires "strumming" the last row of frets).
                                    // When false: tapping mode (notes trigger on any fret touch)

    // Advanced timing parameters (in milliseconds)
    uint32_t state_snapshot_window_ms;
    uint32_t fret_stale_timeout_ms;
    uint32_t fret_very_recent_threshold_ms;
    uint32_t fret_post_strum_threshold_ms;
    uint32_t fret_release_delay_ms;

    bool dirty;                     // Used to determine whether a flash write is required
    bool low_batt;                  // Low battery detected
    int8_t preset_selected;         // Selected preset number (-1 = null, 0-3 = preset 0-3)
} state_data_t;

typedef enum amy_fx {
    REVERB,
    FILTER,
    CHORUS,
    ECHO,
    DISTORTION,
} amy_fx_t;

state_data_t* get_state_data(void);

uint8_t get_patch();
void set_patch(uint8_t value);
void set_patch_up();
void set_patch_down();

context_t get_context();
void set_context(context_t value);

selection_t get_selection();
void set_selection(selection_t value);
void set_selection_up();
void set_selection_down();

bool get_fx(amy_fx_t fx);
void set_fx(amy_fx_t fx, bool value);

// Reverb parameters
float get_reverb_liveness();
void set_reverb_liveness(float value);
void set_reverb_liveness_up();
void set_reverb_liveness_down();

float get_reverb_damping();
void set_reverb_damping(float value);
void set_reverb_damping_up();
void set_reverb_damping_down();

float get_reverb_xover_hz();
void set_reverb_xover_hz(float value);
void set_reverb_xover_hz_up();
void set_reverb_xover_hz_down();
void reset_reverb_fx();

// Chorus parameters
int get_chorus_max_delay();
void set_chorus_max_delay(int value);
void set_chorus_max_delay_up();
void set_chorus_max_delay_down();

float get_chorus_lfo_freq();
void set_chorus_lfo_freq(float value);
void set_chorus_lfo_freq_up();
void set_chorus_lfo_freq_down();

float get_chorus_depth();
void set_chorus_depth(float value);
void set_chorus_depth_up();
void set_chorus_depth_down();
void reset_chorus_fx();

// Echo parameters
float get_echo_delay_ms();
void set_echo_delay_ms(float value);
void set_echo_delay_ms_up();
void set_echo_delay_ms_down();

float get_echo_feedback();
void set_echo_feedback(float value);
void set_echo_feedback_up();
void set_echo_feedback_down();

float get_echo_filter_coef();
void set_echo_filter_coef(float value);
void set_echo_filter_coef_up();
void set_echo_filter_coef_down();
void reset_echo_fx();

// Filter parameters (LPF24 only)
float get_filter_freq_hz();
void set_filter_freq_hz(float value);
void set_filter_freq_hz_up();
void set_filter_freq_hz_down();

float get_filter_resonance();
void set_filter_resonance(float value);
void set_filter_resonance_up();
void set_filter_resonance_down();
void reset_filter_fx();

// Distortion parameters
float get_distortion_level();
void set_distortion_level(float value);
void set_distortion_level_up();
void set_distortion_level_down();

float get_distortion_gain();
void set_distortion_gain(float value);
void set_distortion_gain_up();
void set_distortion_gain_down();
void reset_distortion_fx();

uint8_t get_volume();
void set_volume(uint8_t value);
void set_volume_up();
void set_volume_down();

uint8_t get_contrast();
void set_contrast(uint8_t value);
void set_contrast_up();
void set_contrast_down();

uint8_t get_string_pitch(uint8_t string);
void set_string_pitch(uint8_t string, uint8_t value);
void set_string_pitch_up(uint8_t string);
void set_string_pitch_down(uint8_t string);

int16_t get_capo();
void set_capo(int16_t value);
void set_capo_up();
void set_capo_down();
void reset_tuning();

bool get_playing_mode();
void set_playing_mode(bool value);
void toggle_playing_mode();

bool get_lefthanded();
void set_lefthanded(bool value);
void toggle_lefthanded();

bool get_dirty();
void set_dirty(bool value);

bool get_low_batt();
void set_low_batt(bool value);

int8_t get_preset_selected();
void set_preset_selected(int8_t value);

// Advanced timing parameters
uint32_t get_state_snapshot_window_ms();
void set_state_snapshot_window_ms(uint32_t value);
void set_state_snapshot_window_ms_up();
void set_state_snapshot_window_ms_down();

uint32_t get_fret_stale_timeout_ms();
void set_fret_stale_timeout_ms(uint32_t value);
void set_fret_stale_timeout_ms_up();
void set_fret_stale_timeout_ms_down();

uint32_t get_fret_very_recent_threshold_ms();
void set_fret_very_recent_threshold_ms(uint32_t value);
void set_fret_very_recent_threshold_ms_up();
void set_fret_very_recent_threshold_ms_down();

uint32_t get_fret_post_strum_threshold_ms();
void set_fret_post_strum_threshold_ms(uint32_t value);
void set_fret_post_strum_threshold_ms_up();
void set_fret_post_strum_threshold_ms_down();

uint32_t get_fret_release_delay_ms();
void set_fret_release_delay_ms(uint32_t value);
void set_fret_release_delay_ms_up();
void set_fret_release_delay_ms_down();

void reset_advanced_timing();

uint8_t get_random_u8();

#ifdef __cplusplus
}
#endif

#endif
