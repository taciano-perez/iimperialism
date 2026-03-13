#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <apple2.h>
#include <tgi.h>
#include "font.h"
#include "pictures.h"

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
    tgi_setcolor (TGI_COLOR_BLACK);
    tgi_bar(5*CHAR_WIDTH, 20*CHAR_HEIGHT, 40*CHAR_WIDTH, 23*CHAR_HEIGHT);
    tgi_setcolor (TGI_COLOR_WHITE);
}

void clear_area(int x, int y, int width, int height) {
    tgi_setcolor (TGI_COLOR_BLACK);
    tgi_bar(x * CHAR_WIDTH, y * CHAR_HEIGHT,
            (x + width) * CHAR_WIDTH - 1,
            (y + height) * CHAR_HEIGHT - 1);
    tgi_setcolor (TGI_COLOR_WHITE);
}

void print (const int x, const int y, const char* text) {
    draw_custom_text (x*CHAR_WIDTH, y*CHAR_HEIGHT, text);
}

void print_right_aligned(const int x, const int y, const char* text) {
    unsigned int text_length;
    int pos_x;
    text_length = strlen(text);
    pos_x = x - text_length + 1;
    if (pos_x < 0) {
        pos_x = 0;
    }
    print(pos_x, y, text);
}

void print_int_right_aligned(int x, int y, unsigned int value) {
    char buffer[10];
    sprintf(buffer, "%u", value);
    print_right_aligned(x, y, buffer);
}

void print_int(const int x, const int y, unsigned int value) {
    char buffer[5];
    sprintf(buffer, "%u", value);
    print(x, y, buffer);
}

void draw_picture_at(const unsigned char picture_index, const unsigned char x_byte, unsigned char y) {
    draw_picture(picture_index, x_byte, y*CHAR_HEIGHT);
}

void box (const int x1, const int y1, const int x2, const int y2) {
    tgi_line (x1*CHAR_WIDTH, y1*CHAR_HEIGHT, x1*CHAR_WIDTH, y2*CHAR_HEIGHT);
    tgi_line (x1*CHAR_WIDTH, y1*CHAR_HEIGHT, x2*CHAR_WIDTH+(CHAR_WIDTH-1), y1*CHAR_HEIGHT);
    tgi_line (x2*CHAR_WIDTH+(CHAR_WIDTH-1), y1*CHAR_HEIGHT, x2*CHAR_WIDTH+(CHAR_WIDTH-1), y2*CHAR_HEIGHT);
    tgi_line (x1*CHAR_WIDTH, y2*CHAR_HEIGHT, x2*CHAR_WIDTH+(CHAR_WIDTH-1), y2*CHAR_HEIGHT);
}

/* Read a single character from the keyboard, echoing it at (x, y). */
char cgetc_at(int x, int y) {
    char ch;
    char buf[2];
    ch = cgetc();
    buf[0] = ch;
    buf[1] = '\0';
    print(x, y, buf);
    return ch;
}

/* Read an unsigned integer from the keyboard, echoing digits at (x, y).
 * max_digits limits input length (1-10). Press Enter to confirm. */
unsigned int scan_uint(int x, int y, unsigned int max_digits) {
    char buffer[11];
    unsigned int len = 0;
    char ch;

    if (max_digits > 10) max_digits = 10;
    buffer[0] = '\0';

    while (1) {
        ch = cgetc();

        if (ch >= '0' && ch <= '9') {
            if (len < max_digits) {
                buffer[len++] = ch;
                buffer[len] = '\0';
                /* Clear field then redraw */
                tgi_setcolor(TGI_COLOR_BLACK);
                tgi_bar(x * CHAR_WIDTH, y * CHAR_HEIGHT,
                        (x + (int)max_digits) * CHAR_WIDTH - 1,
                        y * CHAR_HEIGHT + CHAR_HEIGHT - 1);
                tgi_setcolor(TGI_COLOR_WHITE);
                print(x, y, buffer);
            }
        } else if (ch == '\b' || ch == 127) { /* backspace or delete */
            if (len > 0) {
                len--;
                buffer[len] = '\0';
                tgi_setcolor(TGI_COLOR_BLACK);
                tgi_bar(x * CHAR_WIDTH, y * CHAR_HEIGHT,
                        (x + (int)max_digits) * CHAR_WIDTH - 1,
                        y * CHAR_HEIGHT + CHAR_HEIGHT - 1);
                tgi_setcolor(TGI_COLOR_WHITE);
                if (len > 0) print(x, y, buffer);
            }
        } else if (ch == '\r' || ch == '\n') {
            if (len > 0) {
                return (unsigned int)atoi(buffer);
            }
        }
    }
}

#pragma code-name (pop)
