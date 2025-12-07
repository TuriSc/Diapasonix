#include "pico/stdlib.h"
#include "config.h"
#include "fretboard.h"
#include "state_data.h"

uint8_t fretboard[NUM_STRINGS][NUM_FRETS] = FRETBOARD_LAYOUT;

// Helper function to clamp MIDI note to valid range
static inline uint8_t clamp_midi_note(int16_t note) {
    if (note < 0) return 0;
    if (note > MIDI_NOTE_MAX) return MIDI_NOTE_MAX;
    return (uint8_t)note;
}

// Get the string and fret of an electrode by id
void get_string_fret(uint8_t id, uint8_t* string, uint8_t* fret) {
    // Initialize to invalid values - will be set if found
    *string = NUM_STRINGS;  // Invalid value (out of bounds)
    *fret = NUM_FRETS;      // Invalid value (out of bounds)
    
    bool lefthanded = get_lefthanded();
    for (uint8_t i = 0; i < NUM_STRINGS; i++) {
        for (uint8_t j = 0; j < NUM_FRETS; j++) {
            if (fretboard[i][j] == id) {
                if (lefthanded) {
                    *string = NUM_STRINGS - 1 - i;
                    *fret = j;
                } else {
                    *string = i;
                    *fret = j;
                }
                return;
            }
        }
    }
}

uint8_t get_string_by_id(uint8_t id) {
    uint8_t string;
    uint8_t fret;
    get_string_fret(id, &string, &fret);
    return string;
}

uint8_t get_fret_by_id(uint8_t id) {
    uint8_t string;
    uint8_t fret;
    get_string_fret(id, &string, &fret);
    return fret;
}

uint8_t get_id_from_string_fret(uint8_t string, uint8_t fret) {
    bool lefthanded = get_lefthanded();
    if (lefthanded) {
        string = NUM_STRINGS - 1 - string;
    }
    return fretboard[string][fret];
}

uint8_t get_note_by_string_fret(uint8_t string, uint8_t fret){
    uint8_t pitch = get_string_pitch(string);
    int16_t capo = get_capo();
    int16_t note = pitch + fret + capo;
    return clamp_midi_note(note);
}

uint8_t get_note_by_id(uint8_t id) {
    uint8_t string;
    uint8_t fret;
    get_string_fret(id, &string, &fret);
    return get_note_by_string_fret(string, fret);
}
