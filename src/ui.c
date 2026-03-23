#include <conio.h>
#include <stdlib.h>
#include <apple2.h>
#include <tgi.h>
#include "font.h"
#include "pictures.h"
#include "ui_buffers.h"

#define CHAR_WIDTH 7
#define CHAR_HEIGHT 8
static char* append_ulong_decimal(char* buffer, unsigned long value);

#pragma code-name (push, "LOWCODE")

static char* append_ulong_decimal(char* buffer, unsigned long value) {
    unsigned long divisor;

    divisor = 1UL;
    while ((value / divisor) >= 10UL) {
        divisor *= 10UL;
    }

    do {
        *buffer++ = (char)('0' + (unsigned char)(value / divisor));
        value %= divisor;
        divisor /= 10UL;
    } while (divisor != 0UL);

    return buffer;
}

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

void format_uint(char* buffer, unsigned int value) {
    append_ulong_decimal(buffer, value)[0] = '\0';
}

void format_money(char* buffer, unsigned long value) {
    if (value >= 1000000UL) {
        unsigned long whole;
        unsigned char fractional;

        whole = value / 1000000UL;
        fractional = (unsigned char)(((value % 1000000UL) * 100UL) / 1000000UL);

        *buffer++ = '$';
        buffer = append_ulong_decimal(buffer, whole);
        *buffer++ = '.';
        *buffer++ = (char)('0' + (fractional / 10U));
        *buffer++ = (char)('0' + (fractional % 10U));
        *buffer++ = 'M';
        *buffer = '\0';
        return;
    }

    *buffer++ = '$';
    append_ulong_decimal(buffer, value)[0] = '\0';
}

void print_int_right_aligned(unsigned char x, unsigned char y, unsigned int value) {
    format_uint(ui_buffer, value);
    print_right_aligned(x, y, ui_buffer);
}

void print_int_right_aligned_currency(unsigned char x, unsigned char y, unsigned long value) {
    format_money(ui_buffer, value);
    print_right_aligned(x, y, ui_buffer);
}

void print_int(unsigned char x, unsigned char y, unsigned int value) {
    format_uint(ui_buffer, value);
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
                clear_area(x, y, max_digits, 1);
                print(x, y, ui_buffer);
            }
        } else if (ch == '\b' || ch == 127) { /* backspace or delete */
            if (len > 0) {
                ui_buffer[--len] = '\0';
                clear_area(x, y, max_digits, 1);
                if (len > 0) {
                    print(x, y, ui_buffer);
                }
            }
        } else if (ch == '\r' || ch == '\n') {
            if (len > 0) {
                return (unsigned int)atoi(ui_buffer);
            }
        }
    }
}

void scan_text(unsigned char x, unsigned char y, char* buffer, unsigned char max_length) {
    unsigned char len = 0;
    char ch;

    if (max_length == 0U) {
        if (buffer != 0) {
            buffer[0] = '\0';
        }
        return;
    }

    if (max_length > 31U) {
        max_length = 31U;
    }

    buffer[0] = '\0';

    while (1) {
        ch = cgetc();

        if (ch >= 32 && ch <= 126) {
            if (len < max_length) {
                buffer[len++] = ch;
                buffer[len] = '\0';
                clear_area(x, y, max_length, 1);
                print_bold(x, y, buffer);
            }
        } else if (ch == '\b' || ch == 127) {
            if (len > 0U) {
                buffer[--len] = '\0';
                clear_area(x, y, max_length, 1);
                if (len > 0U) {
                    print_bold(x, y, buffer);
                }
            }
        } else if (ch == '\r' || ch == '\n') {
            if (len > 0U) {
                return;
            }
        }
    }
}

#pragma code-name (pop)
