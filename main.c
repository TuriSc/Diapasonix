/* Diapasonix
 * Portable electronic instrument and Midi controller
 * with capacitive touchpad fretboard, built-in synth and speaker.
 * By Turi Scandurra – https://turiscandurra.com/circuits
 */
 
#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/structs/clocks.h"
#include "pico/multicore.h"
#include "hardware/adc.h"   // Used for low battery detection
#include "pico/binary_info.h"

#include "amy.h"

#include "config.h"
#include "audio/audio_i2s.h"
#include "multicore_audio.h"
#include "ssd1306.h"
#include "battery-check.h"

#include "global_filter.h"
#include "global_distortion.h"
#include "state_data.h"
#include "touch.h"
#include "flash.h"
#include "fretboard.h"
#include "display/display.h"
#include "directional_switch.h"

#if defined (USE_MIDI)
#include "bsp/board_api.h"  // For TinyUSB Midi
#include "tusb.h"           // For TinyUSB Midi
#endif

/* Globals */

state_data_t* state_data;

static alarm_id_t power_on_alarm_id;
struct audio_buffer_pool *ap;

ssd1306_t display;

amy_event e[NUM_STRINGS];

#if defined (USE_MIDI)
/* MIDI helper function */
static inline uint32_t tud_midi_write24 (uint8_t jack_id, uint8_t b1, uint8_t b2, uint8_t b3) {
    // Use static array to avoid stack corruption when called from multiple contexts
    // But ensure thread-safety by copying values immediately
    uint8_t msg[3];
    msg[0] = b1;
    msg[1] = b2;
    msg[2] = b3;
    
    // Validate MIDI note range before sending
    if(b1 == MIDI_NOTE_ON || b1 == MIDI_NOTE_OFF) {  // Note on or note off
        if(b2 > MIDI_NOTE_MAX) {
            // Invalid note - don't send
            return 0;
        }
    }
    
    return tud_midi_stream_write(jack_id, msg, 3);
}
#endif

/* Note and audio */

int64_t power_on_complete() {
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    return 0;
}

void note_on(uint8_t string, uint8_t note) {
    // Validate string is within bounds
    if(string >= NUM_STRINGS) {
        return;  // Invalid string index
    }
    
    // Validate note is within MIDI range
    if(note > MIDI_NOTE_MAX) {
        return;  // Invalid note
    }
    
    float velocity = 0.5;
    e[string] = amy_default_event();
    e[string].time = 0;
    e[string].synth = string;  // Each string is its own instrument
    e[string].midi_note = note;
    e[string].velocity = velocity;
    amy_add_event(&e[string]);
    
#if defined (USE_MIDI)
    // Send MIDI note on message (MIDI_NOTE_ON = note on, channel 0)
    // Double-check note value before sending to MIDI
    uint8_t midi_note = note;
    if(midi_note > MIDI_NOTE_MAX) {
        midi_note = MIDI_NOTE_MAX;  // Clamp to valid range
    }
    
    uint8_t midi_velocity = (uint8_t)(velocity * MIDI_NOTE_MAX);
    tud_midi_write24(0, MIDI_NOTE_ON, midi_note, midi_velocity);
#endif
}

void note_off(uint8_t string, uint8_t note) {
    if(string >= NUM_STRINGS) {
        return;
    }
    
    e[string] = amy_default_event();
    e[string].time = 0;
    e[string].synth = string;
    e[string].midi_note = note;
    e[string].velocity = 0;  // velocity = 0 means note off
    amy_add_event(&e[string]);
    
#if defined (USE_MIDI)
    // Send MIDI note off message (MIDI_NOTE_OFF = note off, channel 0)
    tud_midi_write24(0, MIDI_NOTE_OFF, note, 0);
#endif
}

void power_on_led() {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    if (power_on_alarm_id > 0) {
        cancel_alarm(power_on_alarm_id);
    }
    power_on_alarm_id = add_alarm_in_ms(500, power_on_complete, NULL, true);
}

void intro_complete() {
    set_context(CTX_MAIN);
    set_selection(SELECTION_PATCH);
    display_draw(&display);
}

// Exported functions
void update_display() {
    if(get_contrast() == CONTRAST_AUTO) {
        display_wake(&display);
    }
    set_draw_pending(true);
}

void update_patch() {
    uint16_t patch_num = get_patch();
    
    // Set up each string as its own instrument
    for(uint8_t i = 0; i < NUM_STRINGS; i++) {
        // Create an event to load the patch for this string/instrument
        e[i] = amy_default_event();
        e[i].time = 0;
        e[i].synth = i;                 // Each string is instrument 0, 1, 2, 3
        e[i].patch_number = patch_num;  // Load the selected patch
        e[i].num_voices = 1;            // Just one voice per string
        amy_add_event(&e[i]);
    }
}

void update_tuning() {
    set_draw_pending(true);
}

void update_volume() {
    // Convert UI volume (0-8) to AMY volume (0-11.0 float)
    uint8_t ui_volume = get_volume();
    float amy_volume;
    
    if (ui_volume == 0) {
        amy_volume = 0.0f;
    } else {
        amy_volume = ((float)ui_volume / 8.0f) * 11.0f;
    }
        
    // Set AMY global volume directly
    amy_global.volume = amy_volume;
}

void update_fx(amy_fx_t fx) {
    switch(fx){
        case REVERB:
            config_reverb((float)get_fx(REVERB)*2.0f, get_reverb_liveness(), get_reverb_damping(), get_reverb_xover_hz());
        break;
        case CHORUS:
            config_chorus((float)get_fx(CHORUS)*2.0f, get_chorus_max_delay(), get_chorus_lfo_freq(), get_chorus_depth());
        break;
        case ECHO:
        {
            // Ensure AMY is running and echo feature is enabled before configuring echo
            if (!amy_global.running || !AMY_HAS_ECHO) {
                break;
            }
            
            float level = (float)get_fx(ECHO)*2.0f;
            
            config_echo(level, get_echo_delay_ms(), DIAPASONIX_ECHO_DEFAULT_MAX_DELAY_MS, get_echo_feedback(), get_echo_filter_coef());
        }
        break;
        case FILTER:
        {
            bool enabled = get_fx(FILTER);
            global_filter_set_enabled(enabled);
            if (enabled) {
                config_global_filter(get_filter_freq_hz(), get_filter_resonance());
            }
        }
        break;
        case DISTORTION:
        {
            bool enabled = get_fx(DISTORTION);
            global_distortion_set_enabled(enabled);
            if (enabled) {
                config_global_distortion(get_distortion_level(), get_distortion_gain());
            }
        }
        break;
    }
}

void initialize_default_settings() {
    // Settings not loaded, initialize state_data with default values
    set_patch(DEFAULT_PATCH);

    set_fx(REVERB, false);
    set_fx(FILTER, false);
    set_fx(CHORUS, false);
    set_fx(ECHO, false);
    set_fx(DISTORTION, false);
    
    // Initialize effect parameters with defaults
    set_reverb_liveness(DIAPASONIX_REVERB_DEFAULT_LIVENESS);
    set_reverb_damping(DIAPASONIX_REVERB_DEFAULT_DAMPING);
    set_reverb_xover_hz(DIAPASONIX_REVERB_DEFAULT_XOVER_HZ);
    
    set_chorus_max_delay(DIAPASONIX_CHORUS_DEFAULT_MAX_DELAY);
    set_chorus_lfo_freq(DIAPASONIX_CHORUS_DEFAULT_LFO_FREQ);
    set_chorus_depth(DIAPASONIX_CHORUS_DEFAULT_MOD_DEPTH);
    
    set_echo_delay_ms(DIAPASONIX_ECHO_DEFAULT_DELAY_MS);
    set_echo_feedback((float)DIAPASONIX_ECHO_DEFAULT_FEEDBACK);
    set_echo_filter_coef((float)DIAPASONIX_ECHO_DEFAULT_FILTER_COEF);
    
    // TODO: Add filter type. The only available filter is LPF24,
    // but AMY also supports HPF, BPF, and LPF.
    set_filter_freq_hz(DIAPASONIX_FILTER_DEFAULT_FREQ_HZ);
    set_filter_resonance(DIAPASONIX_FILTER_DEFAULT_RESONANCE);
    
    set_distortion_level(DIAPASONIX_DISTORTION_DEFAULT_LEVEL);
    set_distortion_gain(DIAPASONIX_DISTORTION_DEFAULT_GAIN);
    
    set_volume(DEFAULT_VOLUME); // 0-8 range, gets converted to AMY's 0-11.0 range
    set_contrast(CONTRAST_AUTO); // Automatic dimming of display brightness

    set_string_pitch(0, DEFAULT_STRING_PITCH_0); // G3
    set_string_pitch(1, DEFAULT_STRING_PITCH_1); // D3
    set_string_pitch(2, DEFAULT_STRING_PITCH_2); // A2
    set_string_pitch(3, DEFAULT_STRING_PITCH_3); // E2

    set_playing_mode(false); // Start in tapping mode
    set_lefthanded(false);
    
    set_preset_selected(-1); // No preset selected initially
    
    // Initialize advanced timing parameters with defaults
    set_state_snapshot_window_ms(STATE_SNAPSHOT_WINDOW_MS);
    set_fret_stale_timeout_ms(FRET_STALE_TIMEOUT_MS);
    set_fret_very_recent_threshold_ms(FRET_VERY_RECENT_THRESHOLD_MS);
    set_fret_post_strum_threshold_ms(FRET_POST_STRUM_THRESHOLD_MS);
    set_fret_release_delay_ms(FRET_RELEASE_DELAY_MS);
}

/* Low battery */
void battery_low_detected() {
    set_low_batt(true);
    battery_check_stop(); // Stop the timer
}

/* Binary information */
void bi_decl_all() {
    bi_decl(bi_program_name(PROGRAM_NAME));
    bi_decl(bi_program_description(PROGRAM_DESCRIPTION));
    bi_decl(bi_program_version_string(PROGRAM_VERSION));
    bi_decl(bi_program_url(PROGRAM_URL));
    bi_decl(bi_1pin_with_name(BATT_LVL_PIN, "Battery sensing pin"));
    
    // I2C pins
    bi_decl(bi_2pins_with_func(SDA_PIN, SCL_PIN, GPIO_FUNC_I2C));
    bi_decl(bi_1pin_with_name(SDA_PIN, "SDA"));
    bi_decl(bi_1pin_with_name(SCL_PIN, "SCL"));

    // I2S audio pins
    bi_decl(bi_1pin_with_name(PICO_AUDIO_I2S_DATA_PIN, "I2S DIN"));
    bi_decl(bi_1pin_with_name(PICO_AUDIO_I2S_CLOCK_PIN_BASE, "I2S BCK"));
    bi_decl(bi_1pin_with_name(PICO_AUDIO_I2S_CLOCK_PIN_BASE + 1, "I2S LRCK"));
    
    // Directional switch pins
    bi_decl(bi_1pin_with_name(DS_A, "DS_A"));
    bi_decl(bi_1pin_with_name(DS_B, "DS_B"));
    bi_decl(bi_1pin_with_name(DS_C, "DS_C"));
    bi_decl(bi_1pin_with_name(DS_D, "DS_D"));
    bi_decl(bi_1pin_with_name(DS_X, "DS_X (Center)"));
}

/* MIDI stubs */
void run_midi() {
    (void)0; // No-op
}

void amy_send_midi_note_on(uint16_t osc) {
    (void)osc; // No-op
}

void amy_send_midi_note_off(uint16_t osc) {
    (void)osc; // No-op
}

int main() {
    // Overclock to 226MHz
    set_sys_clock_pll(VCO_FREQ, PLL_PD1, PLL_PD2); 

    stdio_init_all();

    // Use the onboard LED as a power-on indicator
    power_on_led();

    printf("%s\n", USB_STR_PRODUCT);
    printf("\n");
    printf("Clock is set to %d\n", clock_get_hz(clk_sys));

    gpio_init(SDA_PIN);
    gpio_init(SCL_PIN);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    i2c_init(I2C_PORT, I2C_FREQ);
    display_init(&display);

    // Initialize the touch modules
    mpr121_initialize();

    bi_decl_all();

    directional_switch_init();

    // Initialize the ADC, used for voltage sensing
    adc_init();
    // Launch the battery check timed task
    battery_check_init(5000, NULL, (void*)battery_low_detected);

    // Set state data
    set_context(CTX_INIT);
    set_dirty(false);
    set_low_batt(false);
    // Attempt to load previous settings, if stored on flash
    bool data_loaded = load_flash_data();
    if(!data_loaded) {
        initialize_default_settings();
    }
    // Clear dirty flag after loading to prevent writing back the same data we just loaded
    set_dirty(false);

    // Initialize AMY synthesis engine
    amy_config_t amy_config = amy_default_config();
    amy_config.audio = AMY_AUDIO_IS_NONE;  // Using custom I2S
    amy_config.features.default_synths = 0;
    amy_config.features.echo = 1;  // Explicitly enable echo feature
    amy_start(amy_config);
    
    // Initialize global filter
    global_filter_init();
    global_distortion_init();
    
    // Wait a little for AMY initialization to complete
    sleep_ms(100);

    // Launch Core1 for multicore audio processing
    multicore_launch_core1(core1_main);

    // Wait for Core1 ready signal
    await_message_from_other_core();
    
    amy_global.running = 1;

    sleep_ms(250);

    // Apply all loaded settings to the audio engine, so
    // effects are properly enabled/configured after loading from flash.
    update_patch();
    update_fx(REVERB);
    update_fx(CHORUS);
    update_fx(ECHO);
    update_fx(FILTER);
    update_fx(DISTORTION);
    update_tuning();
    update_volume();

    // Init i2s audio
    ap = init_audio();

    // Show a short intro animation. This will distract the user
    // while the hardware is calibrating.
    play_intro_animation(&display, intro_complete);
    set_context(CTX_MAIN);
    set_selection(SELECTION_PATCH);
    display_draw(&display);

#if defined (USE_MIDI)
    // Enable Midi device functionality
    board_init();
    tud_init(BOARD_TUD_RHPORT);

    if (board_init_after_tusb) {
        board_init_after_tusb();
    }
#endif

    // Apply settings that were set at startup
    update_patch();
    update_volume();

    while(true) { // Main loop
        // Perform any pending flash writes before audio operations
        // to ensures Core1 is not reset while audio is being processed.
        flash_write_task();
        
        directional_switch_task(); // Poll buttons

        mpr121_task();

        delay_ms(5); // Amy's idle function
        display_task(&display); // Poll display updates after audio as display I2C can be such a block
        
#if defined (USE_MIDI)
        tud_task(); // TinyUSB device task
#endif
    }
    return 0;
}
