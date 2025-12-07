#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "mpr121.h"         // https://github.com/antgon/pico-mpr121
#include "config.h"
#include "touch.h"
#include "state_data.h"
#include "fretboard.h"

struct mpr121_sensor mpr121;
struct mpr121_sensor mpr121_1;

bool touched[NUM_STRINGS][NUM_FRETS];

// Track which note is currently playing on each string
static uint8_t playing_note[NUM_STRINGS] = {0, 0, 0, 0};
static bool note_is_playing[NUM_STRINGS] = {false, false, false, false};
static bool is_open_string[NUM_STRINGS] = {false, false, false, false};
static bool playing_mode_mode_last[NUM_STRINGS] = {false, false, false, false}; // Track mode changes
static uint32_t last_strum_time[NUM_STRINGS] = {0, 0, 0, 0}; // Track when each string was last strummed
static uint32_t open_string_release_time[NUM_STRINGS] = {0}; // Track when strum fret was released for open string notes

// Helper function to read touched status register
static inline uint16_t read_touched_status(struct mpr121_sensor *sensor) {
    uint8_t reg = 0x00; // MPR121_TOUCH_STATUS_REG
    uint8_t vals[2];
    i2c_write_blocking(sensor->i2c_port, sensor->i2c_addr, &reg, 1, true);
    i2c_read_blocking(sensor->i2c_port, sensor->i2c_addr, vals, 2, false);
    return (vals[1] << 8 | vals[0]) & 0x0fff;
}

void mpr121_initialize(){
    mpr121_init(I2C_PORT, MPR121_ADDRESS, &mpr121);
    mpr121_init(I2C_PORT, MPR121_ADDRESS_1, &mpr121_1);

    mpr121_set_thresholds(MPR121_TOUCH_THRESHOLD,
                          MPR121_RELEASE_THRESHOLD, &mpr121);
    mpr121_set_thresholds(MPR121_TOUCH_THRESHOLD,
                          MPR121_RELEASE_THRESHOLD, &mpr121_1);

    mpr121_enable_electrodes(12, &mpr121);
    mpr121_enable_electrodes(12, &mpr121_1);

    mpr121_write(MPR121_SOFT_RESET_REG, 0x63, &mpr121);
    mpr121_write(MPR121_SOFT_RESET_REG, 0x63, &mpr121_1);

    for (int i = 0; i < 12; i++) {
        mpr121_write(MPR121_TOUCH_THRESHOLD_REG + i * 2, MPR121_TOUCH_THRESHOLD, &mpr121);
        mpr121_write(MPR121_RELEASE_THRESHOLD_REG + i * 2, MPR121_RELEASE_THRESHOLD, &mpr121);

        mpr121_write(MPR121_TOUCH_THRESHOLD_REG + i * 2, MPR121_TOUCH_THRESHOLD, &mpr121_1);
        mpr121_write(MPR121_RELEASE_THRESHOLD_REG + i * 2, MPR121_RELEASE_THRESHOLD, &mpr121_1);
    }

    mpr121_write(MPR121_ELECTRODE_CONFIG_REG, 0x8C, &mpr121);
    mpr121_write(MPR121_ELECTRODE_CONFIG_REG, 0x8C, &mpr121_1);
    
    // Optimize filter delay counts for lower latency
    // Reduce falling filter delay from 0x02 to 0x00 for faster release detection
    // Enter stop mode to write configuration registers
    mpr121_write(MPR121_ELECTRODE_CONFIG_REG, 0x00, &mpr121);
    mpr121_write(MPR121_FILTER_DELAY_COUNT_FALLING_REG, 0x00, &mpr121);
    mpr121_write(MPR121_ELECTRODE_CONFIG_REG, 0x8C, &mpr121);  // Back to run mode
    
    mpr121_write(MPR121_ELECTRODE_CONFIG_REG, 0x00, &mpr121_1);
    mpr121_write(MPR121_FILTER_DELAY_COUNT_FALLING_REG, 0x00, &mpr121_1);
    mpr121_write(MPR121_ELECTRODE_CONFIG_REG, 0x8C, &mpr121_1);  // Back to run mode
    
    // Optionally reduce second filter iterations (SFI) from 4 to 2 samples for faster response
    // This reduces latency but may slightly increase noise sensitivity
    // Current FILTER_CONFIG_REG is 0x20 (ESI=1ms, SFI=4, CDT=0.5μs)
    // Change to 0x18 (ESI=1ms, SFI=2, CDT=0.5μs) for faster filtering
    // Uncomment the lines below to enable this optimization:
    // mpr121_write(MPR121_ELECTRODE_CONFIG_REG, 0x00, &mpr121);  // Enter stop mode
    // mpr121_write(MPR121_FILTER_CONFIG_REG, 0x18, &mpr121);     // Reduce SFI to 2 samples
    // mpr121_write(MPR121_ELECTRODE_CONFIG_REG, 0x8C, &mpr121);   // Back to run mode
    // mpr121_write(MPR121_ELECTRODE_CONFIG_REG, 0x00, &mpr121_1);
    // mpr121_write(MPR121_FILTER_CONFIG_REG, 0x18, &mpr121_1);
    // mpr121_write(MPR121_ELECTRODE_CONFIG_REG, 0x8C, &mpr121_1);
    
    // Initialize playing mode tracking
    bool initial_playing_mode = get_playing_mode();
    for(uint8_t i = 0; i < NUM_STRINGS; i++) {
        playing_mode_mode_last[i] = initial_playing_mode;
    }
}

// Track when a fret was released for release delay logic.
// Must be declared before process_delayed_note_offs() function uses it.
static uint32_t fret_release_delay_time[NUM_STRINGS][NUM_FRETS] = {0};

// Forward declarations for helper functions used in process_delayed_note_offs
static void stop_note_on_string(uint8_t string);
static bool same_note_still_available(uint8_t string, uint8_t released_fret, uint8_t released_note);

// Process delayed note-offs and open string sustain timeout
static void process_delayed_note_offs(void) {
    uint32_t now = time_us_32() / 1000;
    uint32_t release_delay = get_fret_release_delay_ms();
    
    for(uint8_t string = 0; string < NUM_STRINGS; string++) {
        // Check open string sustain timeout
        if(open_string_release_time[string] > 0 && note_is_playing[string] && is_open_string[string]) {
            uint32_t sustain_elapsed = now - open_string_release_time[string];
            if(sustain_elapsed >= OPEN_STRING_SUSTAIN_TIME_MS) {
                // Open string sustain timeout expired. Time to stop the note.
                stop_note_on_string(string);
                open_string_release_time[string] = 0;
            }
        }
        
        // Check delayed note-offs for fretted notes
        if(release_delay > 0) {
            for(uint8_t fret = 0; fret < NUM_FRETS - 1; fret++) {
                if(fret_release_delay_time[string][fret] > 0) {
                    uint32_t delay_elapsed = now - fret_release_delay_time[string][fret];
                    if(delay_elapsed >= release_delay) {
                        // Delay expired - check if note should still be turned off.
                        // Only turn off if no other fret is currently touched that would play the same note.
                        if(note_is_playing[string] && !is_open_string[string]) {
                            uint8_t released_fret_note = 1 + get_note_by_string_fret(string, fret);
                            if(playing_note[string] == released_fret_note) {
                                if(!same_note_still_available(string, fret, released_fret_note)) {
                                    stop_note_on_string(string);
                                }
                            }
                        }
                        // Clear the delay time
                        fret_release_delay_time[string][fret] = 0;
                    }
                }
            }
        }
    }
}

void mpr121_task(){
    static bool was_touched[24];
    
    // Process delayed note-offs first
    process_delayed_note_offs();
    
    uint16_t touched_status_0 = read_touched_status(&mpr121);
    uint16_t touched_status_1 = read_touched_status(&mpr121_1);
    
    // Process first sensor (electrodes 0-11)
    for(uint8_t i=0; i<12; i++) {
        bool is_touched = (touched_status_0 >> i) & 1;
        if (is_touched != was_touched[i]){
            if (is_touched){
                touch_on(i);
            } else {
                touch_off(i);
            }
            was_touched[i] = is_touched;
        }
    }
    
    // Process second sensor (electrodes 12-23)
    for(uint8_t i=0; i<12; i++) {
        uint8_t j = i + 12;
        bool is_touched = (touched_status_1 >> i) & 1;
        if (is_touched != was_touched[j]){
            if (is_touched){
                touch_on(j);
            } else {
                touch_off(j);
            }
            was_touched[j] = is_touched;
        }
    }
}

uint16_t get_touched() {
    uint16_t touched;
    mpr121_touched(&touched, &mpr121);
    return touched;
}

// Debouncing: track last touch event time per electrode
static uint32_t last_touch_time[24] = {0};

// Track release times for frets to implement release tolerance
// When a fret is released, we still consider it "touched" for FRET_RELEASE_TOLERANCE_MS
// This allows natural guitar playing where you lift slightly before strumming
static uint32_t fret_release_time[NUM_STRINGS][NUM_FRETS] = {0};

// State history: Track when each fret was last touched/released
// This allows us to look back in time when strumming to determine intent
static uint32_t fret_touch_time[NUM_STRINGS][NUM_FRETS] = {0};  // When fret was last touched (0 = never or cleared)

// Helper function to check if any frets (including strum fret) are touched on a string
static bool any_fret_touched(uint8_t string) {
    for(uint8_t f = 0; f < NUM_FRETS; f++) {
        if(touched[string][f]) {
            return true;
        }
    }
    return false;
}

// Helper function to stop note on a string
static void stop_note_on_string(uint8_t string) {
    if(note_is_playing[string]) {
        note_off(string, playing_note[string]);
        note_is_playing[string] = false;
        playing_note[string] = 0;
        is_open_string[string] = false;
        open_string_release_time[string] = 0; // Clear open string sustain timer
    }
}

// Helper function to play a note on a string
static void play_note_on_string(uint8_t string, uint8_t note, bool is_open) {
    // Only turn off previous note if it's different
    if(note_is_playing[string] && playing_note[string] != note) {
        note_off(string, playing_note[string]);
    }
    
    note_on(string, note);
    playing_note[string] = note;
    note_is_playing[string] = true;
    is_open_string[string] = is_open;
}

// Check if another fret would play the same note
static bool same_note_still_available(uint8_t string, uint8_t released_fret, uint8_t released_note) {
    // Check currently touched frets
    for(uint8_t i = 0; i < NUM_FRETS - 1; i++) {
        if(i != released_fret && touched[string][i]) {
            uint8_t other_fret_note = 1 + get_note_by_string_fret(string, i);
            if(other_fret_note == released_note) {
                return true;
            }
        }
    }
    
    // Check frets in release tolerance
    uint32_t now_check = time_us_32() / 1000;
    uint32_t release_tolerance_same = FRET_RELEASE_TOLERANCE_MS;
    for(uint8_t i = 0; i < NUM_FRETS - 1; i++) {
        if(i != released_fret && fret_release_time[string][i] > 0) {
            uint32_t time_since_release = now_check - fret_release_time[string][i];
            if(time_since_release < release_tolerance_same) {
                uint8_t tolerance_fret_note = 1 + get_note_by_string_fret(string, i);
                if(tolerance_fret_note == released_note) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

// Build a snapshot of frets that should be considered "touched" at strum time.
// This includes currently touched frets and recently released frets within tolerance.
static void build_fret_snapshot(uint8_t string, uint32_t now, bool snapshot_touched[NUM_FRETS - 1]) {
    // Initialize snapshot array to false
    for(uint8_t k = 0; k < NUM_FRETS - 1; k++) {
        snapshot_touched[k] = false;
    }
    
    // Find the most recent touch time across all frets on this string.
    // This helps determine if a release is "stale" (happened before the most recent touch).
    // For currently touched frets, use their touch time. For released frets, use their release time.
    uint32_t most_recent_touch = 0;
    for(uint8_t i = 0; i < NUM_FRETS - 1; i++) {
        if(touched[string][i]) {
            // Currently touched fret - use its touch time
            if(fret_touch_time[string][i] > most_recent_touch) {
                most_recent_touch = fret_touch_time[string][i];
            }
        } else if(fret_release_time[string][i] > 0) {
            // Released fret - use its release time (when it was last active)
            if(fret_release_time[string][i] > most_recent_touch) {
                most_recent_touch = fret_release_time[string][i];
            }
        } else if(fret_touch_time[string][i] > 0) {
            // Fret was touched but not released yet (shouldn't happen, but handle it)
            if(fret_touch_time[string][i] > most_recent_touch) {
                most_recent_touch = fret_touch_time[string][i];
            }
        }
    }
    
    // Check if any frets are currently touched
    bool any_currently_touched = false;
    for(uint8_t i = 0; i < NUM_FRETS - 1; i++) {
        if(touched[string][i]) {
            any_currently_touched = true;
            break;
        }
    }
    
    for(uint8_t i = 0; i < NUM_FRETS - 1; i++) {
        bool in_snapshot = false;
        
        // Check 1: Currently touched - highest priority (always included)
        if(touched[string][i]) {
            in_snapshot = true;
        }
        // Check 2: Was touched but not released yet
        else if(fret_touch_time[string][i] > 0 && 
               (fret_release_time[string][i] == 0 || fret_release_time[string][i] < fret_touch_time[string][i])) {
            // Not released yet - include it if touch was recent enough
            uint32_t time_since_touch = now - fret_touch_time[string][i];
            // Use a reasonable maximum window to prevent stale values from being included.
            // If touch_time is very old, it's likely stale and should be ignored.
            uint32_t snapshot_window = get_state_snapshot_window_ms();
            uint32_t stale_timeout = get_fret_stale_timeout_ms();
            if(time_since_touch < snapshot_window * 2 && time_since_touch < stale_timeout) {
                in_snapshot = true;
            } else {
                // Touch time is too old, clear it to prevent future issues
                fret_touch_time[string][i] = 0;
            }
        }
        // Check 3: Was released - only exclude if a newer fret is currently touched
        else if(fret_touch_time[string][i] > 0 && fret_release_time[string][i] > 0) {
            uint32_t time_since_release = now - fret_release_time[string][i];
            
            // Check if there's a currently touched fret that was touched after this release.
            // Only exclude released frets if a newer fret is actively touched.
            bool newer_fret_touched = false;
            for(uint8_t j = 0; j < NUM_FRETS - 1; j++) {
                if(j != i && touched[string][j]) {
                    // If the other fret was touched after this one was released, exclude this released fret
                    if(fret_touch_time[string][j] > fret_release_time[string][i]) {
                        newer_fret_touched = true;
                        break;
                    }
                }
            }
            
            // Only include released frets if:
            // 1. Release was very recent (within tolerance)
            // 2. No newer fret is currently touched (avoids stale releases)
            // 3. Release time is not too old (prevent stale values)
            // Note: We don't check against most_recent_touch here because if there's a currently touched fret,
            // it will be prioritized in the selection logic anyway
            uint32_t release_tolerance = FRET_RELEASE_TOLERANCE_MS;
            uint32_t stale_timeout_check = get_fret_stale_timeout_ms();
            if(time_since_release < release_tolerance && 
               time_since_release < stale_timeout_check &&
               !newer_fret_touched) {
                in_snapshot = true;
            } else {
                // Release is too old or invalid - clear both times to prevent future issues
                uint32_t stale_timeout_clear = get_fret_stale_timeout_ms();
                if(time_since_release >= stale_timeout_clear) {
                    fret_touch_time[string][i] = 0;
                    fret_release_time[string][i] = 0;
                }
            }
        }
        
        snapshot_touched[i] = in_snapshot;
    }
}

// Find the best fret from the snapshot to play.
// Returns true if a fret was found, false for open string.
static bool find_best_fret_from_snapshot(uint8_t string, uint32_t now, const bool snapshot_touched[NUM_FRETS - 1], uint8_t *best_fret) {
    bool found_fret = false;
    uint8_t best = 0;
    uint32_t best_touch_time = 0;
    bool best_is_currently_touched = false;
    
    // Find all frets in snapshot
    for(uint8_t i = 0; i < NUM_FRETS - 1; i++) {
        if(snapshot_touched[i]) {
            bool is_currently_touched = touched[string][i];
            // Use actual touch time, not 'now', so we can compare which was touched more recently
            uint32_t touch_time = fret_touch_time[string][i];
            if(touch_time == 0) {
                // Shouldn't happen, but fallback to now if touch_time is 0
                touch_time = now;
            }
            
            // Calculate how long this fret has been touched
            uint32_t touch_duration = now - touch_time;
            uint32_t best_touch_duration = found_fret ? (now - best_touch_time) : 0;
            
            // Prioritize: 1) Currently touched frets (always win over released ones)
            //             2) Among currently touched frets, prefer higher fret number (higher pitch)
            //             3) Very recently touched (within threshold) and currently touched (tie-breaker for same fret state)
            //             4) Most recently touched (for released frets or tie-breaker)
            uint32_t very_recent_threshold = get_fret_very_recent_threshold_ms();
            bool is_very_recent = touch_duration < very_recent_threshold;
            bool best_is_very_recent = found_fret && (best_touch_duration < very_recent_threshold);
            
            if(!found_fret) {
                best_touch_time = touch_time;
                best = i;
                best_is_currently_touched = is_currently_touched;
                found_fret = true;
            } else {
                // Currently touched frets always win over released frets
                if(is_currently_touched && !best_is_currently_touched) {
                    best_touch_time = touch_time;
                    best = i;
                    best_is_currently_touched = is_currently_touched;
                }
                // If both are currently touched, prioritize higher fret number (higher pitch).
                // This matches real stringed instrument behavior: pressing multiple frets plays the highest one.
                else if(is_currently_touched && best_is_currently_touched) {
                    // Higher fret number wins (higher pitch)
                    if(i > best) {
                        best_touch_time = touch_time;
                        best = i;
                        best_is_currently_touched = is_currently_touched;
                    }
                    // If same fret number (shouldn't happen, but handle it), use recency as tie-breaker
                    else if(i == best && touch_time > best_touch_time) {
                        best_touch_time = touch_time;
                        best = i;
                        best_is_currently_touched = is_currently_touched;
                    }
                }
                // If both are released (not currently touched), pick the most recently touched
                else if(!is_currently_touched && !best_is_currently_touched) {
                    if(touch_time > best_touch_time) {
                        best_touch_time = touch_time;
                        best = i;
                        best_is_currently_touched = is_currently_touched;
                    }
                }
            }
        }
    }
    
    if(found_fret && best < NUM_FRETS - 1) {
        *best_fret = best;
        return true;
    }
    
    return false;
}

// Handle strumming mode touch on regular fret (not strum fret)
static void handle_playing_mode_regular_fret_touch(uint8_t string, uint8_t fret) {
    uint32_t now = time_us_32() / 1000;
    
    // Check if this fret was touched within the post-strum threshold
    // If so, treat it as if it was touched before the strum (hammer-on behavior)
    if(last_strum_time[string] > 0) {
        uint32_t time_since_strum = now - last_strum_time[string];
        uint32_t post_strum_threshold = get_fret_post_strum_threshold_ms();
        if(time_since_strum <= post_strum_threshold) {
            // Within threshold: treat as hammer-on - play the note immediately
            uint8_t note = 1 + get_note_by_string_fret(string, fret);
            if(note <= MIDI_NOTE_MAX) {
                // Stop any currently playing note and play the new one
                stop_note_on_string(string);
                open_string_release_time[string] = 0; // Cancel open string sustain
                play_note_on_string(string, note, false);
                return; // Don't dampen, we've already played the note
            }
        }
    }
    
    // Outside threshold or no recent strum: check if we should stop open string note.
    // Stop open string note if a regular fret is touched while strum fret is not touched.
    if(note_is_playing[string] && is_open_string[string]) {
        bool strum_fret_touched = touched[string][NUM_FRETS - 1];
        if(!strum_fret_touched) {
            // Regular fret touched but strum fret not touched. Stop open string note
            stop_note_on_string(string);
            open_string_release_time[string] = 0;
        }
    }
    // Just track that the fret is touched. The note will only play when the strum fret is touched.
}

// Handle strumming mode touch on strum fret
static void handle_playing_mode_strum_fret_touch(uint8_t string) {
    uint32_t now = time_us_32() / 1000;
    
    // Record the strum time for this string
    last_strum_time[string] = now;
    
    // Create a snapshot of the fretboard state at strum time
    // to prevent rapid state changes from causing ambiguity
    bool snapshot_touched[NUM_FRETS - 1];
    build_fret_snapshot(string, now, snapshot_touched);
    
    // Find the best fret in the snapshot
    uint8_t best_fret;
    if(find_best_fret_from_snapshot(string, now, snapshot_touched, &best_fret)) {
        // Calculate the note to play
        uint8_t note = 1 + get_note_by_string_fret(string, best_fret);
        
        // Validate MIDI note range
        if(note > MIDI_NOTE_MAX) {
            return;
        }
        
        play_note_on_string(string, note, false);
        
        // Cancel any pending delayed note-offs since we're playing a new note
        for(uint8_t i = 0; i < NUM_FRETS - 1; i++) {
            fret_release_delay_time[string][i] = 0;
        }
        
        // Cancel open string sustain timer since we're playing a fretted note
        open_string_release_time[string] = 0;
        
        // If this was a recently-released fret that's now being used, clear the release time
        // to prevent it from being considered touched again after the tolerance expires
        if(!touched[string][best_fret]) {
            fret_release_time[string][best_fret] = 0;
        }
    } else {
        // Open string
        uint8_t note = get_note_by_string_fret(string, 0);
        
        // Validate MIDI note range
        if(note > MIDI_NOTE_MAX) {
            return;
        }
        
        play_note_on_string(string, note, true);
        
        // Cancel any pending delayed note-offs since we're playing a new note
        for(uint8_t i = 0; i < NUM_FRETS - 1; i++) {
            fret_release_delay_time[string][i] = 0;
        }
        
        // Reset open string release time - will be set when strum fret is released
        open_string_release_time[string] = 0;
    }
}

// Handle tapping mode touch
static void handle_tapping_touch(uint8_t string, uint8_t id) {
    uint8_t note = get_note_by_id(id);
    
    // Validate MIDI note range
    if(note > MIDI_NOTE_MAX) {
        return;
    }
    
    play_note_on_string(string, note, false);
}

// Clear release times for other frets on a string when a new fret is touched
static void clear_stale_release_times(uint8_t string, uint8_t fret, uint32_t now) {
    for(uint8_t i = 0; i < NUM_FRETS - 1; i++) {
        if(i != fret && touched[string][i]) {
            // Only currently touched frets or very recent releases are considered
            if(fret_release_time[string][i] > 0) {
                uint32_t time_since_release = now - fret_release_time[string][i];
                // Only clear if release was more than tolerance ago.
                // If within tolerance, we might still want to use it.
                uint32_t release_tolerance_clear = FRET_RELEASE_TOLERANCE_MS;
                if(time_since_release >= release_tolerance_clear) {
                    fret_release_time[string][i] = 0;
                }
            }
        }
    }
}

void touch_on(uint8_t id) {
    uint8_t string = get_string_by_id(id);
    uint8_t fret = get_fret_by_id(id);
    
    // Validate string and fret are within bounds
    if(string >= NUM_STRINGS || fret >= NUM_FRETS) {
        return;  // Invalid touch ID
    }
    
    // Check if strumming mode changed. If so, turn off all notes on this string
    bool current_playing_mode = get_playing_mode();
    if(playing_mode_mode_last[string] != current_playing_mode) {
        stop_note_on_string(string);
        playing_mode_mode_last[string] = current_playing_mode;
    }
    
    // Debouncing - prevent spurious rapid triggers
    uint32_t now = time_us_32() / 1000;
    if(now - last_touch_time[id] < MPR121_DEBOUNCE_TIME_MS) {
        return;  // Too soon, ignore
    }
    last_touch_time[id] = now;
    
    touched[string][fret] = true;
    
    // Update state history
    fret_touch_time[string][fret] = now;
    
    // If this fret was recently released and is now touched again, clear the release time
    // This cancels any pending release
    if(fret != NUM_FRETS - 1) {
        if(fret_release_time[string][fret] > 0) {
            fret_release_time[string][fret] = 0;
        }
        
        // Clear release delay time if this fret was delayed
        if(fret_release_delay_time[string][fret] > 0) {
            fret_release_delay_time[string][fret] = 0;
        }
        
        // When a new fret is touched, check if we need to cancel delayed note-offs for other frets.
        // If a new fret is touched quickly after release, cancel the delayed note-off.
        uint32_t release_delay = get_fret_release_delay_ms();
        if(release_delay > 0) {
            for(uint8_t i = 0; i < NUM_FRETS - 1; i++) {
                if(i != fret && fret_release_delay_time[string][i] > 0) {
                    uint32_t delay_elapsed = now - fret_release_delay_time[string][i];
                    if(delay_elapsed < release_delay) {
                        // Cancel the delayed note-off since a new fret was touched
                        fret_release_delay_time[string][i] = 0;
                    }
                }
            }
        }
        
        // When a new fret is touched, clear release times for other frets on this string
        clear_stale_release_times(string, fret, now);
    }
    
    if(get_playing_mode()) {
        // In strumming mode, notes should only play when both a fret and the strum fret are touched.
        // If pressing a regular fret (not the strum fret), just track it - don't play any note yet.
        if(fret != NUM_FRETS - 1) {
            handle_playing_mode_regular_fret_touch(string, fret);
            return;
        }
        
        // Continue only if the touched fret is in the last column (strum fret).
        // At this point, fret must be NUM_FRETS - 1.
        handle_playing_mode_strum_fret_touch(string);
    } else {
        // Tapping mode
        handle_tapping_touch(string, id);
    }
}

// Check if any frets are within release tolerance
static bool any_fret_in_release_tolerance(uint8_t string, uint32_t now) {
    uint32_t release_tolerance_check = FRET_RELEASE_TOLERANCE_MS;
    for(uint8_t i = 0; i < NUM_FRETS - 1; i++) {
        if(fret_release_time[string][i] > 0) {
            uint32_t time_since_release = now - fret_release_time[string][i];
            if(time_since_release < release_tolerance_check) {
                return true;
            }
        }
    }
    return false;
}

// Handle strumming mode release of strum fret
static void handle_playing_mode_strum_fret_release(uint8_t string) {
    touched[string][NUM_FRETS - 1] = false;
    
    // Check if any regular frets are still touched
    bool any_regular_fret_touched = false;
    for(uint8_t i = 0; i < NUM_FRETS - 1; i++) {
        if(touched[string][i]) {
            any_regular_fret_touched = true;
            break;
        }
    }
    
    // If no regular frets are touched and no frets are in release tolerance,
    // handle note stopping based on whether it's an open string or fretted note.
    if(!any_regular_fret_touched) {
        uint32_t now_check = time_us_32() / 1000;
        if(!any_fret_in_release_tolerance(string, now_check) && note_is_playing[string]) {
            if(is_open_string[string]) {
                // Open string note: start sustain timer instead of stopping immediately
                open_string_release_time[string] = now_check;
            } else {
                // Fretted note: stop immediately
                stop_note_on_string(string);
            }
        }
    }
}


// Handle strumming mode release of regular fret
static void handle_playing_mode_regular_fret_release(uint8_t string, uint8_t fret) {
    // Mark the release time before clearing touched state
    // so we can still detect it during strumming.
    uint32_t now = time_us_32() / 1000;
    fret_release_time[string][fret] = now;
    fret_touch_time[string][fret] = 0;  // Clear touch time to indicate it's released
    
    // Clear release times for other frets on this string if they're currently touched
    // to prevent stale release times from interfering with rapid fret changes
    for(uint8_t i = 0; i < NUM_FRETS - 1; i++) {
        if(i != fret && touched[string][i]) {
            // Another fret is currently touched, clear any stale release times.
            // Currently touched fret takes priority.
            fret_release_time[string][i] = 0;
            fret_release_delay_time[string][i] = 0;  // Clear delay time too
        }
    }
    
    // Clear the touched state after marking release time
    // so release tolerance can still detect it.
    touched[string][fret] = false;
    
    // Check if the released fret corresponds to the currently playing note
    // If so, delay turning off the note if release_delay is set.
    if(note_is_playing[string] && !is_open_string[string]) {
        // For fretted notes, calculate what note this fret would produce (with +1 offset)
        uint8_t released_fret_note = 1 + get_note_by_string_fret(string, fret);
        
        // Check if this fret corresponds to the currently playing note
        if(playing_note[string] == released_fret_note) {
            // Check if any other fret would play the same note
            if(!same_note_still_available(string, fret, released_fret_note)) {
                uint32_t release_delay = get_fret_release_delay_ms();
                if(release_delay > 0) {
                    // Delay turning off the note - mark the release delay time
                    fret_release_delay_time[string][fret] = now;
                } else {
                    // No delay - turn off immediately
                    stop_note_on_string(string);
                }
            }
        }
    }
    
    // Check if all frets (including strum fret) are now released.
    // If so, turn off the note to prevent hanging MIDI notes.
    if(!any_fret_touched(string)) {
        bool strum_fret_touched = touched[string][NUM_FRETS - 1];
        if(!strum_fret_touched) {
            uint32_t now_check = time_us_32() / 1000;
            if(!any_fret_in_release_tolerance(string, now_check) && note_is_playing[string]) {
                stop_note_on_string(string);
            }
        }
    }
}

// Handle tapping mode release
static void handle_tapping_release(uint8_t string, uint8_t id) {
    uint8_t released_fret = get_fret_by_id(id);
    touched[string][released_fret] = false;
    
    uint8_t released_note = get_note_by_id(id);
    
    // In tapping mode, frets are independent - only stop the note if it matches the released fret
    if(note_is_playing[string] && playing_note[string] == released_note) {
        // The released note is currently playing - stop it
        stop_note_on_string(string);
    } else {
        // Different note is playing (from another fret that was touched later) - just send note_off for released note
        // and to keep MIDI state clean without messing with the currently playing note.
        note_off(string, released_note);
    }
}

void touch_off(uint8_t id) {
    uint8_t string = get_string_by_id(id);
    uint8_t fret = get_fret_by_id(id);
    
    // Validate string and fret are within bounds
    if(string >= NUM_STRINGS || fret >= NUM_FRETS) {
        return;  // Invalid touch ID
    }
    
    // Debouncing for release events.
    // However, if a note is playing, we should always send note_off to prevent hanging MIDI notes.
    uint32_t now = time_us_32() / 1000;
    bool should_debounce = (now - last_touch_time[id] < MPR121_DEBOUNCE_TIME_MS);
    
    // If debouncing but a note is playing, we still need to send note_off.
    // Check if we have a note playing that needs to be turned off.
    bool note_needs_off = false;
    if(get_playing_mode()) {
        // In strumming mode, check if releasing strum fret
        if(fret == NUM_FRETS - 1) {
            note_needs_off = note_is_playing[string];
        }
    } else {
        // In tapping mode, always need to turn off the note
        note_needs_off = true;
    }
    
    // Only skip if debouncing AND no note needs to be turned off
    if(should_debounce && !note_needs_off) {
        return;  // Too soon and no critical note_off needed, ignore
    }
    
    last_touch_time[id] = now;
    
    if(get_playing_mode()) {
        if(fret == NUM_FRETS - 1) {
            handle_playing_mode_strum_fret_release(string);
        } else {
            handle_playing_mode_regular_fret_release(string, fret);
        }
    } else {
        // Tapping mode
        handle_tapping_release(string, id);
    }
}
