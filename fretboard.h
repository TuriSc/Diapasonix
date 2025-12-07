#ifndef FRETBOARD_H_
#define FRETBOARD_H_

#ifdef __cplusplus
extern "C" {
#endif

void get_string_fret(uint8_t id, uint8_t* string, uint8_t* fret);
uint8_t get_string_by_id(uint8_t id);
uint8_t get_fret_by_id(uint8_t id);
uint8_t get_id_from_string_fret(uint8_t string, uint8_t fret);
uint8_t get_note_by_string_fret(uint8_t string, uint8_t fret);
uint8_t get_note_by_id(uint8_t id);

#ifdef __cplusplus
}
#endif

#endif