#ifndef CONFIG_H_
#define CONFIG_H_

#define PROGRAM_NAME                "Diapasonix"
#define PROGRAM_VERSION             "1.0.0"
#define PROGRAM_DESCRIPTION         "Portable musical instrument and Midi controller"
#define PROGRAM_URL                 "https://turiscandurra.com/circuits"
#define USB_STR_MANUFACTURER        "TuriScandurra"
#define USB_STR_PRODUCT             "Diapasonix"

#define USE_MIDI                    // Comment out this line to disable Midi output
                                    // and enable USB stdio.

/* MIDI constants */
#define MIDI_NOTE_MAX               127
#define MIDI_NOTE_ON                0x90
#define MIDI_NOTE_OFF               0x80

/* Clock values */
#define F_CPU                       225000000
#define F_CPU_MOD_KHZ               226000
#define F_SAMP                      44140
#define VCO_FREQ                    1356000000
#define PLL_PD1                     6
#define PLL_PD2                     1

/* Fretboard */
#define NUM_STRINGS                 4
#define NUM_FRETS                   6
// Define the electrode matrix. You might have to
// adjust this according to your own layout
#define FRETBOARD_LAYOUT { \
                                        {12, 13, 14, 15, 16, 17}, \
                                        {18, 19, 20, 21, 22, 23}, \
                                        {11, 10,  9,  8,  7,  6}, \
                                        { 5,  4,  3,  2,  1,  0} \
}

#define PICO_DEFAULT_FLOAT_IMPL         pico
#define USE_AUDIO_I2S                   1
#define PICO_AUDIO_I2S_DATA_PIN         28 // I2S DIN
#define PICO_AUDIO_I2S_CLOCK_PIN_BASE   26 // I2S BCK
                                           // 27 is LRCK

/* AMY synth */
#define DEFAULT_PATCH              226
#define DEFAULT_VOLUME             3 // 0-8 range, gets converted to AMY's 0-11.0 range

/* String pitch defaults */
#define DEFAULT_STRING_PITCH_0     55 // G3
#define DEFAULT_STRING_PITCH_1     50 // D3
#define DEFAULT_STRING_PITCH_2     45 // A2
#define DEFAULT_STRING_PITCH_3     40 // E2

/* Reverb defaults */
#define DIAPASONIX_REVERB_DEFAULT_LIVENESS     0.85f
#define DIAPASONIX_REVERB_DEFAULT_DAMPING      0.5f
#define DIAPASONIX_REVERB_DEFAULT_XOVER_HZ     3000.0f

/* Chorus defaults */
#define DIAPASONIX_CHORUS_DEFAULT_MAX_DELAY    320
#define DIAPASONIX_CHORUS_DEFAULT_LFO_FREQ     0.5f
#define DIAPASONIX_CHORUS_DEFAULT_MOD_DEPTH    0.5f

/* Echo/Delay defaults */
#define DIAPASONIX_ECHO_DEFAULT_DELAY_MS       150.0f
#define DIAPASONIX_ECHO_DEFAULT_FEEDBACK       0.75f
#define DIAPASONIX_ECHO_DEFAULT_FILTER_COEF    0.0f
#define DIAPASONIX_ECHO_DEFAULT_MAX_DELAY_MS   200.0f  // 200ms requires ~65KB for 2 channels (vs 131KB for 400ms).
                                                       // If the delay is too long, you will run out of RAM and
                                                       // the program will crash.

/* Filter defaults */
#define DIAPASONIX_FILTER_DEFAULT_FREQ_HZ      1000.0f  // 1 kHz cutoff
#define DIAPASONIX_FILTER_DEFAULT_RESONANCE    0.7f     // Default Q factor

/* Distortion defaults */
#define DIAPASONIX_DISTORTION_DEFAULT_LEVEL    0.75f    // 0.0 to 1.0 - amount of distortion
#define DIAPASONIX_DISTORTION_DEFAULT_GAIN     10.0f    // 10.0 to 20.0 internally (displayed as 1.0 to 2.0 in UI) - drive/gain before distortion

/* I2C */
#define I2C_PORT                    i2c0
#define SDA_PIN                     4 // i2c0
#define SCL_PIN                     5 // i2c0
#define I2C_FREQ                    400 * 1000 // 400kHz

/* MPR121 */
#define MPR121_ADDRESS              0x5A
#define MPR121_ADDRESS_1            0x5C // ADDR tied to SDA. Make sure your module is not shorting ADD to GND.
#define MPR121_TOUCH_THRESHOLD      22   // These values might have to be adjusted based on the
#define MPR121_RELEASE_THRESHOLD    16   // size of your electrodes and their distance from the chip.
                                         // Threshold range is 0-255. If set incorrectly, you will get
                                         // "ghost" note_on and note_off events.
#define MPR121_DEBOUNCE_TIME_MS     20   // Ignore touches within 20ms of previous events.

/* SSD1306 */
#define SSD1306_ADDRESS             0x3C
#define SSD1306_WIDTH               128
#define SSD1306_HEIGHT              64
#define SSD1306_ROTATION            1    // Rotate the display by 90°

/* Directional switch pin assignment */
#define DS_A                        6
#define DS_B                        7
#define DS_C                        8
#define DS_D                        9
#define DS_X                        10   // Center switch

#define LONG_PRESS_THRESHOLD        750  // Amount of ms to hold the button to trigger a long press

/* Touch state tolerances */
#define FRET_RELEASE_TOLERANCE_MS   25   // When a fret is released, still consider it touched for this many ms.
                                         // This allows natural guitar-like playing where you lift slightly
                                         // before strumming.

#define STATE_SNAPSHOT_WINDOW_MS    100  // When strumming, look back this many ms to determine which frets were touched.
                                         // This helps capture the "intent" when rapidly switching frets.

#define FRET_STALE_TIMEOUT_MS       5000 // Maximum age for touch/release times before they're considered stale.
                                         // Used to prevent very old touch events from affecting current state.

#define FRET_VERY_RECENT_THRESHOLD_MS 50 // Threshold for considering a touch "very recent".
                                         // Used for prioritizing very recent touches in fret selection.

#define FRET_POST_STRUM_THRESHOLD_MS 30  // Threshold after a strum where frets pressed are treated
                                         // as if they were pressed before the strum (hammer-on behavior).
                                         // Outside this threshold, frets will dampen the string normally.

#define OPEN_STRING_SUSTAIN_TIME_MS 3000  // Time that open string notes continue playing after
                                          // strum fret is released in strumming mode. The note stops when:
                                          // - A new note is played on the same string, OR
                                          // - A regular fret is touched while strum fret is not touched, OR
                                          // - This timeout expires.

#define FRET_RELEASE_DELAY_MS       50   // Threshold for delaying note off when descending quickly

/* Display dimming */
#define DISPLAY_DIM_DELAY           5     // Seconds since last UI interaction for the screen
                                          // to be dimmed when contrast is set to AUTO

/* Battery level pin */
#define BATT_LVL_PIN                29

/* Flash memory */
#define USE_FLASH_STORAGE           true // Set to false to disable flash storage (both reading and writing)
#define FLASH_TARGET_OFFSET         (FLASH_SECTOR_SIZE * 511)
                                        // Reserve the last 4KB of the default 2MB flash for persistence.
#define MAGIC_NUMBER                {0x44, 0x50, 0x53, 0x58} // 'DPSX' - Diapasonix magic number
#define MAGIC_NUMBER_LENGTH         4
#define FLASH_WRITE_DELAY_S         10  // To minimize flash operations, delay writing by this amount of seconds.
                                        // Unfortunately, the audio output is interrupted for a very short instant 
                                        // during write operations.

/* Presets */
#define NUM_PRESETS                 4

/* Default preset values - Preset 0 */
// Presets are 0-3 internally but 1-4 on the UI
#define PRESET_0_PATCH              DEFAULT_PATCH
#define PRESET_0_FX_REVERB          0
#define PRESET_0_FX_FILTER          0
#define PRESET_0_FX_CHORUS          0
#define PRESET_0_FX_ECHO            0
#define PRESET_0_FX_DISTORTION      0
#define PRESET_0_REVERB_LIVENESS    DIAPASONIX_REVERB_DEFAULT_LIVENESS
#define PRESET_0_REVERB_DAMPING     DIAPASONIX_REVERB_DEFAULT_DAMPING
#define PRESET_0_REVERB_XOVER_HZ    DIAPASONIX_REVERB_DEFAULT_XOVER_HZ
#define PRESET_0_CHORUS_MAX_DELAY   DIAPASONIX_CHORUS_DEFAULT_MAX_DELAY
#define PRESET_0_CHORUS_LFO_FREQ    DIAPASONIX_CHORUS_DEFAULT_LFO_FREQ
#define PRESET_0_CHORUS_DEPTH       DIAPASONIX_CHORUS_DEFAULT_MOD_DEPTH
#define PRESET_0_ECHO_DELAY_MS      DIAPASONIX_ECHO_DEFAULT_DELAY_MS
#define PRESET_0_ECHO_FEEDBACK      DIAPASONIX_ECHO_DEFAULT_FEEDBACK
#define PRESET_0_ECHO_FILTER_COEF   DIAPASONIX_ECHO_DEFAULT_FILTER_COEF
#define PRESET_0_FILTER_FREQ_HZ     DIAPASONIX_FILTER_DEFAULT_FREQ_HZ
#define PRESET_0_FILTER_RESONANCE   DIAPASONIX_FILTER_DEFAULT_RESONANCE
#define PRESET_0_DISTORTION_LEVEL   DIAPASONIX_DISTORTION_DEFAULT_LEVEL
#define PRESET_0_DISTORTION_GAIN    DIAPASONIX_DISTORTION_DEFAULT_GAIN
#define PRESET_0_STRING_PITCH_0     DEFAULT_STRING_PITCH_0
#define PRESET_0_STRING_PITCH_1     DEFAULT_STRING_PITCH_1
#define PRESET_0_STRING_PITCH_2     DEFAULT_STRING_PITCH_2
#define PRESET_0_STRING_PITCH_3     DEFAULT_STRING_PITCH_3
#define PRESET_0_CAPO               0
#define PRESET_0_PLAYING_MODE       0   // Tapping mode

/* Default preset values - Preset 1 */
#define PRESET_1_PATCH              241
#define PRESET_1_FX_REVERB          1
#define PRESET_1_FX_FILTER          0
#define PRESET_1_FX_CHORUS          0
#define PRESET_1_FX_ECHO            0
#define PRESET_1_FX_DISTORTION      0
#define PRESET_1_REVERB_LIVENESS    DIAPASONIX_REVERB_DEFAULT_LIVENESS
#define PRESET_1_REVERB_DAMPING     DIAPASONIX_REVERB_DEFAULT_DAMPING
#define PRESET_1_REVERB_XOVER_HZ    DIAPASONIX_REVERB_DEFAULT_XOVER_HZ
#define PRESET_1_CHORUS_MAX_DELAY   DIAPASONIX_CHORUS_DEFAULT_MAX_DELAY
#define PRESET_1_CHORUS_LFO_FREQ    DIAPASONIX_CHORUS_DEFAULT_LFO_FREQ
#define PRESET_1_CHORUS_DEPTH       DIAPASONIX_CHORUS_DEFAULT_MOD_DEPTH
#define PRESET_1_ECHO_DELAY_MS      DIAPASONIX_ECHO_DEFAULT_DELAY_MS
#define PRESET_1_ECHO_FEEDBACK      DIAPASONIX_ECHO_DEFAULT_FEEDBACK
#define PRESET_1_ECHO_FILTER_COEF   DIAPASONIX_ECHO_DEFAULT_FILTER_COEF
#define PRESET_1_FILTER_FREQ_HZ     DIAPASONIX_FILTER_DEFAULT_FREQ_HZ
#define PRESET_1_FILTER_RESONANCE   DIAPASONIX_FILTER_DEFAULT_RESONANCE
#define PRESET_1_DISTORTION_LEVEL   DIAPASONIX_DISTORTION_DEFAULT_LEVEL
#define PRESET_1_DISTORTION_GAIN    DIAPASONIX_DISTORTION_DEFAULT_GAIN
#define PRESET_1_STRING_PITCH_0     DEFAULT_STRING_PITCH_0
#define PRESET_1_STRING_PITCH_1     DEFAULT_STRING_PITCH_1
#define PRESET_1_STRING_PITCH_2     DEFAULT_STRING_PITCH_2
#define PRESET_1_STRING_PITCH_3     DEFAULT_STRING_PITCH_3
#define PRESET_1_CAPO               0
#define PRESET_1_PLAYING_MODE       0   // Tapping mode

/* Default preset values - Preset 2 */
#define PRESET_2_PATCH              40
#define PRESET_2_FX_REVERB          0
#define PRESET_2_FX_FILTER          0
#define PRESET_2_FX_CHORUS          1
#define PRESET_2_FX_ECHO            0
#define PRESET_2_FX_DISTORTION      0
#define PRESET_2_REVERB_LIVENESS    DIAPASONIX_REVERB_DEFAULT_LIVENESS
#define PRESET_2_REVERB_DAMPING     DIAPASONIX_REVERB_DEFAULT_DAMPING
#define PRESET_2_REVERB_XOVER_HZ    DIAPASONIX_REVERB_DEFAULT_XOVER_HZ
#define PRESET_2_CHORUS_MAX_DELAY   DIAPASONIX_CHORUS_DEFAULT_MAX_DELAY
#define PRESET_2_CHORUS_LFO_FREQ    DIAPASONIX_CHORUS_DEFAULT_LFO_FREQ
#define PRESET_2_CHORUS_DEPTH       DIAPASONIX_CHORUS_DEFAULT_MOD_DEPTH
#define PRESET_2_ECHO_DELAY_MS      DIAPASONIX_ECHO_DEFAULT_DELAY_MS
#define PRESET_2_ECHO_FEEDBACK      DIAPASONIX_ECHO_DEFAULT_FEEDBACK
#define PRESET_2_ECHO_FILTER_COEF   DIAPASONIX_ECHO_DEFAULT_FILTER_COEF
#define PRESET_2_FILTER_FREQ_HZ     DIAPASONIX_FILTER_DEFAULT_FREQ_HZ
#define PRESET_2_FILTER_RESONANCE   DIAPASONIX_FILTER_DEFAULT_RESONANCE
#define PRESET_2_DISTORTION_LEVEL   DIAPASONIX_DISTORTION_DEFAULT_LEVEL
#define PRESET_2_DISTORTION_GAIN    DIAPASONIX_DISTORTION_DEFAULT_GAIN
#define PRESET_2_STRING_PITCH_0     DEFAULT_STRING_PITCH_0
#define PRESET_2_STRING_PITCH_1     DEFAULT_STRING_PITCH_1
#define PRESET_2_STRING_PITCH_2     DEFAULT_STRING_PITCH_2
#define PRESET_2_STRING_PITCH_3     DEFAULT_STRING_PITCH_3
#define PRESET_2_CAPO               0
#define PRESET_2_PLAYING_MODE       0   // Tapping mode

/* Default preset values - Preset 3 */
#define PRESET_3_PATCH              239
#define PRESET_3_FX_REVERB          0
#define PRESET_3_FX_FILTER          0
#define PRESET_3_FX_CHORUS          0
#define PRESET_3_FX_ECHO            1
#define PRESET_3_FX_DISTORTION      0
#define PRESET_3_REVERB_LIVENESS    DIAPASONIX_REVERB_DEFAULT_LIVENESS
#define PRESET_3_REVERB_DAMPING     DIAPASONIX_REVERB_DEFAULT_DAMPING
#define PRESET_3_REVERB_XOVER_HZ    DIAPASONIX_REVERB_DEFAULT_XOVER_HZ
#define PRESET_3_CHORUS_MAX_DELAY   DIAPASONIX_CHORUS_DEFAULT_MAX_DELAY
#define PRESET_3_CHORUS_LFO_FREQ    DIAPASONIX_CHORUS_DEFAULT_LFO_FREQ
#define PRESET_3_CHORUS_DEPTH       DIAPASONIX_CHORUS_DEFAULT_MOD_DEPTH
#define PRESET_3_ECHO_DELAY_MS      DIAPASONIX_ECHO_DEFAULT_DELAY_MS
#define PRESET_3_ECHO_FEEDBACK      DIAPASONIX_ECHO_DEFAULT_FEEDBACK
#define PRESET_3_ECHO_FILTER_COEF   DIAPASONIX_ECHO_DEFAULT_FILTER_COEF
#define PRESET_3_FILTER_FREQ_HZ     DIAPASONIX_FILTER_DEFAULT_FREQ_HZ
#define PRESET_3_FILTER_RESONANCE   DIAPASONIX_FILTER_DEFAULT_RESONANCE
#define PRESET_3_DISTORTION_LEVEL   DIAPASONIX_DISTORTION_DEFAULT_LEVEL
#define PRESET_3_DISTORTION_GAIN    DIAPASONIX_DISTORTION_DEFAULT_GAIN
#define PRESET_3_STRING_PITCH_0     DEFAULT_STRING_PITCH_0
#define PRESET_3_STRING_PITCH_1     DEFAULT_STRING_PITCH_1
#define PRESET_3_STRING_PITCH_2     DEFAULT_STRING_PITCH_2
#define PRESET_3_STRING_PITCH_3     DEFAULT_STRING_PITCH_3
#define PRESET_3_CAPO               0
#define PRESET_3_PLAYING_MODE       0   // Tapping mode

#endif /* CONFIG_H_ */
