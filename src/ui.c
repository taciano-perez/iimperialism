#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <apple2.h>
#include <tgi.h>
#include "font.h"
#include "pictures.h"
#include "ui_buffers.h"

#define CHAR_WIDTH 7
#define CHAR_HEIGHT 8

#pragma code-name (push, "LOWCODE")

void ui_init() {
    tgi_install (a2_hi_tgi);
    tgi_init ();
    tgi_setcolor (TGI_COLOR_WHITE);
}

void ui_exit() {
    tgi_uninstall ();
}

void clear_screen() {
    tgi_clear();
}

void clear_input_area() {
    clear_area(5, 20, 34, 3);
}

void clear_area(unsigned char x, unsigned char y, unsigned char width, unsigned char height) {
    tgi_setcolor (TGI_COLOR_BLACK);
    tgi_bar(x * CHAR_WIDTH, y * CHAR_HEIGHT,
            (x + width) * CHAR_WIDTH - 1,
            (y + height) * CHAR_HEIGHT - 1);
    tgi_setcolor (TGI_COLOR_WHITE);
}

void paint_area(unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned char color) {
    tgi_setcolor (color);
    tgi_bar(x * CHAR_WIDTH, y * CHAR_HEIGHT,
            (x + width) * CHAR_WIDTH - 1,
            (y + height) * CHAR_HEIGHT - 1);
    tgi_setcolor (TGI_COLOR_WHITE);
}

void print(unsigned char x, unsigned char y, const char* text) {
    draw_text_hgr_opaque(text, x, y);
}

void print_bold(unsigned char x, unsigned char y, const char* text) {
    draw_text_hgr_opaque_bold(text, x, y);
}

void print_right_aligned(unsigned char x, unsigned char y, const char* text) {
    unsigned char text_length;
    int pos_x;
    text_length = (unsigned char)strlen(text);
    pos_x = x - text_length + 1;
    if (pos_x < 0) {
        pos_x = 0;
    }
    print((unsigned char)pos_x, y, text);
}

void print_int_right_aligned(unsigned char x, unsigned char y, unsigned int value) {
    sprintf(ui_buffer, "%u", value);
    print_right_aligned(x, y, ui_buffer);
}

void print_int_right_aligned_currency(unsigned char x, unsigned char y, unsigned long value) {
    if (value >= 1000000UL) {
        unsigned long whole = value / 1000000UL;
        unsigned char fractional = (unsigned char)(((value % 1000000UL) * 100UL) / 1000000UL);
        sprintf(ui_buffer, "$%lu.%02uM", whole, fractional);
    } else {
        sprintf(ui_buffer, "$%lu", value);
    }
    print_right_aligned(x, y, ui_buffer);
}

void print_int(unsigned char x, unsigned char y, unsigned int value) {
    sprintf(ui_buffer, "%u", value);
    print(x, y, ui_buffer);
}

void draw_picture_at(const unsigned char picture_index, const unsigned char x_byte, unsigned char y) {
    draw_picture(picture_index, x_byte, y*CHAR_HEIGHT);
}

void box(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2) {
    tgi_line (x1*CHAR_WIDTH, y1*CHAR_HEIGHT, x1*CHAR_WIDTH, y2*CHAR_HEIGHT); // left
    tgi_line (x1*CHAR_WIDTH, y1*CHAR_HEIGHT-1, x2*CHAR_WIDTH+(CHAR_WIDTH-1), y1*CHAR_HEIGHT-1); // top
    tgi_line (x2*CHAR_WIDTH+(CHAR_WIDTH-1), y1*CHAR_HEIGHT, x2*CHAR_WIDTH+(CHAR_WIDTH-1), y2*CHAR_HEIGHT); // right
    tgi_line (x1*CHAR_WIDTH, y2*CHAR_HEIGHT, x2*CHAR_WIDTH+(CHAR_WIDTH-1), y2*CHAR_HEIGHT); // bottom
}

/* Read a single character from the keyboard, echoing it at (x, y). */
char cgetc_at(unsigned char x, unsigned char y) {
    char ch;
    ui_buffer[0] = ch = cgetc();
    ui_buffer[1] = '\0';
    print(x, y, ui_buffer);
    return ch;
}

/* Read an unsigned integer from the keyboard, echoing digits at (x, y).
 * max_digits limits input length (1-10). Press Enter to confirm. */
unsigned int scan_uint(unsigned char x, unsigned char y, unsigned char max_digits) {
    unsigned char len = 0;
    char ch;

    if (max_digits > 10) max_digits = 10;
    ui_buffer[0] = '\0';

    while (1) {
        ch = cgetc();

        if (ch >= '0' && ch <= '9') {
            if (len < max_digits) {
                ui_buffer[len++] = ch;
                ui_buffer[len] = '\0';
                /* Clear field then redraw */
                tgi_setcolor(TGI_COLOR_BLACK);
                tgi_bar(x * CHAR_WIDTH, y * CHAR_HEIGHT,
                        (x + max_digits) * CHAR_WIDTH - 1,
                        y * CHAR_HEIGHT + CHAR_HEIGHT - 1);
                tgi_setcolor(TGI_COLOR_WHITE);
                print(x, y, ui_buffer);
            }
        } else if (ch == '\b' || ch == 127) { /* backspace or delete */
            if (len > 0) {
                ui_buffer[--len] = '\0';
                tgi_setcolor(TGI_COLOR_BLACK);
                tgi_bar(x * CHAR_WIDTH, y * CHAR_HEIGHT,
                        (x + max_digits) * CHAR_WIDTH - 1,
                        y * CHAR_HEIGHT + CHAR_HEIGHT - 1);
                tgi_setcolor(TGI_COLOR_WHITE);
                if (len > 0) print(x, y, ui_buffer);
            }
        } else if (ch == '\r' || ch == '\n') {
            if (len > 0) {
                return (unsigned int)atoi(ui_buffer);
            }
        }
    }
}

#pragma code-name (pop)
