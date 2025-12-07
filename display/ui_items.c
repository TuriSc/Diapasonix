#include "pico/stdlib.h"
#include "ssd1306.h"
#include "ui_items.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

inline static int16_t count_digits(uint16_t n) {
    if (n > -100) return 2;
    if (n > -10) return 1;
    if (n < 10) return 1;
    if (n < 100) return 2;
    return 3;
}

void draw_rounded_rect(ssd1306_t *p, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t r) {
    uint16_t x1 = x + r;
    uint16_t y1 = y + r;
    uint16_t x2 = x + width - r - 1;
    uint16_t y2 = y + height - r - 1;

    ssd1306_draw_square(p, x1, y, width - r * 2, height);
    ssd1306_draw_square(p, x, y1, width, height - r * 2);

    ssd1306_draw_circle(p, x1, y1, r);
    ssd1306_draw_circle(p, x2, y1, r);
    ssd1306_draw_circle(p, x2, y2, r);
    ssd1306_draw_circle(p, x1, y2, r);
}

void draw_empty_rounded_rect(ssd1306_t *p, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t r) {
    draw_rounded_rect(p, x, y, width, height, r);
    
    uint16_t r1 = r - 1;
    uint16_t x1 = x + r;
    uint16_t y1 = y + r;
    uint16_t x2 = x + width - r - 1;
    uint16_t y2 = y + height - r - 1;

    ssd1306_clear_square(p, x+4, y+1, width-8, height-2);

    ssd1306_clear_circle(p, x1, y1, r1);
    ssd1306_clear_circle(p, x2, y1, r1);
    ssd1306_clear_circle(p, x2, y2, r1);
    ssd1306_clear_circle(p, x1, y2, r1);
}

void draw_entry(ssd1306_t *p, uint8_t y, const char *s, bool selected) {
    if(selected) {
        draw_rounded_rect(p, 0, y, 64, 12, 6);
        ssd1306_clear_string(p, 3, y + 2, 1, s);
    } else {
        draw_empty_rounded_rect(p, 0, y, 64, 12, 6);
        ssd1306_draw_string(p, 3, y + 2, 1, s);
    }
}

void draw_entry_value(ssd1306_t *p, uint8_t y, const char *s, bool selected, uint8_t value) {
    char str[10];
    if(selected) {
        uint8_t x = 34 - 5 * count_digits(value);
        snprintf(str, sizeof(str), "%d", value);
        draw_rounded_rect(p, 0, y, 64, 12, 6);
        ssd1306_clear_string(p, x, y + 2, 1, str);
        ssd1306_clear_string(p, 3, y + 2, 1, "<");
        ssd1306_clear_string(p, 55, y + 2, 1, ">");
    } else {
        snprintf(str, sizeof(str), "%s %d", s, value);
        draw_empty_rounded_rect(p, 0, y, 64, 12, 6);
        ssd1306_draw_string(p, 3, y + 2, 1, str);
    }
}

void draw_entry_value_signed(ssd1306_t *p, uint8_t y, const char *s, bool selected, int16_t value) {
    char str[10];
    if(selected) {
        uint8_t x = 34 - 5 * count_digits(value);
        snprintf(str, sizeof(str), "%s%d", (value > 0) ? "+" : "", value);
        draw_rounded_rect(p, 0, y, 64, 12, 6);
        ssd1306_clear_string(p, x, y + 2, 1, str);
        ssd1306_clear_string(p, 3, y + 2, 1, "<");
        ssd1306_clear_string(p, 55, y + 2, 1, ">");
    } else {
        snprintf(str, sizeof(str), "%s %s%d", s, (value > 0) ? "+" : "", value);
        draw_empty_rounded_rect(p, 0, y, 64, 12, 6);
        ssd1306_draw_string(p, 3, y + 2, 1, str);
    }
}

void draw_entry_value_string(ssd1306_t *p, uint8_t y, const char *s, bool selected, const char *value_str) {
    char str[32];
    if(selected) {
        uint8_t value_len = strlen(value_str);
        uint8_t x = 32 - (value_len * 3);
        if (x < 8) x = 8;
        draw_rounded_rect(p, 0, y, 64, 12, 6);
        ssd1306_clear_string(p, x, y + 2, 1, value_str);
        ssd1306_clear_string(p, 3, y + 2, 1, "<");
        ssd1306_clear_string(p, 55, y + 2, 1, ">");
    } else {
        snprintf(str, sizeof(str), "%s %s", s, value_str);
        draw_empty_rounded_rect(p, 0, y, 64, 12, 6);
        ssd1306_draw_string(p, 3, y + 2, 1, str);
    }
}

void draw_entry_pitch(ssd1306_t *p, uint8_t y, const char *s, bool selected) {
    char str[10];
    bool diesis = (strlen(s) == 2);
    snprintf(str, sizeof(str), "%s%s%s", "----", s, (diesis ? "-----" : "----"));

    if(selected) {
        draw_rounded_rect(p, 0, y, 64, 12, 6);
        ssd1306_clear_string(p, 2, y + 2, 1, str);
        ssd1306_clear_string(p, 3, y + 2, 1, "<");
        ssd1306_clear_string(p, 55, y + 2, 1, ">");
    } else {
        draw_empty_rounded_rect(p, 0, y, 64, 12, 6);
        ssd1306_draw_string(p, 2, y + 2, 1, str);
    }
}

void draw_entry_radio(ssd1306_t *p, uint8_t y, const char *s, bool selected, bool value) {
    uint8_t x1 = 57;
    uint8_t y1 = y + 5;
    if(selected) {
        draw_rounded_rect(p, 0, y, 64, 12, 6);
        ssd1306_clear_string(p, 3, y + 2, 1, s);
        if(value) {
            ssd1306_clear_circle(p, x1, y1, 5);
            ssd1306_draw_circle(p, x1, y1, 4);
        } else {
            ssd1306_clear_circle(p, x1, y1, 4);
        }
    } else {
        draw_empty_rounded_rect(p, 0, y, 64, 12, 6);
        ssd1306_draw_string(p, 3, y + 2, 1, s);
        ssd1306_draw_circle(p, x1, y1, 4);
        if(!value) {
            ssd1306_clear_circle(p, x1, y1, 3);
        }
    }
    
}

void draw_entry_ab(ssd1306_t *p, uint8_t y, const char *s0, const char *s1, bool selected, bool value){
    if(selected) {
        draw_rounded_rect(p, 0, y, 64, 12, 6);
        ssd1306_clear_string(p, 3, y + 2, 1, (value ? s1 : s0));
    } else {
        draw_empty_rounded_rect(p, 0, y, 64, 12, 6);
        ssd1306_draw_string(p, 3, y + 2, 1, (value ? s1 : s0));
    }
}

uint8_t draw_multiline_string(ssd1306_t *p, uint8_t x, uint8_t y, const char *s, uint8_t line_height) {
    uint8_t max_chars = 10;
    uint8_t len = strlen(s);
    uint8_t num_substr = (len + max_chars - 1) / max_chars;
    char **substrings = malloc(num_substr * sizeof(char *));
    uint8_t c = 0;

    for (uint8_t i = 0; i < len; i += max_chars) {
        uint8_t substr_len = (i + max_chars <= len) ? max_chars : len - i;
        substrings[c] = malloc((substr_len + 1) * sizeof(char));
        strncpy(substrings[c], s + i, substr_len);
        substrings[c][substr_len] = '\0'; // Terminate the substring
        c++; // Not that other language!
    }

    // Print the lines to screen
    for (uint8_t i = 0; i < num_substr; i++) {
        ssd1306_draw_string(p, x, y, 1, substrings[i]);
        y += line_height;
        free(substrings[i]);
    }
    free(substrings);

    return y;
}
