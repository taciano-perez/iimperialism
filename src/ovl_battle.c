#include <conio.h>
#include <stdlib.h>
#include <tgi.h>
#include "game.h"

#define FALSE 0
#define TRUE 1

#define state (*s)

static void get_ship_position(unsigned char is_enemy,
                              unsigned char ship_index,
                              unsigned char* x_offset,
                              unsigned char* y_offset) {
    static const unsigned char player_ship_columns[3] = { 0, 6, 12 };
    static const unsigned char pirate_ship_columns[3] = { 23, 29, 35 };
    const unsigned char* ship_columns;

    ship_columns = is_enemy ? pirate_ship_columns : player_ship_columns;
    *x_offset = ship_columns[ship_index % 3];
    *y_offset = 2 + ((ship_index / 3) * 4);
}

static void animate_ship_hit(unsigned char is_enemy, unsigned char x_offset, unsigned char y_offset) {
    unsigned char picture;
    unsigned char i;

    picture = is_enemy ? SHIP_PIRATE : SHIP;

    for (i = 0; i < 6; ++i) {
        paint_area(x_offset, y_offset, 4, 3, TGI_COLOR_WHITE);
        draw_picture_at(picture, x_offset, y_offset);
    }
}

void render_battle_screen(GameState *s) {
    unsigned int i;
    unsigned char visible_friendly_ships;
    unsigned char visible_enemy_ships;

    clear_screen();

    visible_friendly_ships = MIN(state.frigates, 12);
    visible_enemy_ships = MIN(state.frigates, 12);

    // Render player's navy
    print(0, 0, "Haxaco Navy");
    print(0, 1, "Firepower:");
    print_int_right_aligned(15, 1, state.frigates * GUNS_PER_FRIGATE);
    for (i = 0; i < visible_friendly_ships; i++) {
        unsigned char x_offset;
        unsigned char y_offset;

        get_ship_position(FALSE, (unsigned char)i, &x_offset, &y_offset);
        draw_picture_at(SHIP, x_offset, y_offset);
    }

    // Render pirate fleet
    print(23, 0, "Pirate Fleet");
    print(23, 1, "Firepower:");
    print_int_right_aligned(38, 1, state.frigates * GUNS_PER_FRIGATE);
    for (i = 0; i < visible_enemy_ships; i++) {
        unsigned char x_offset;
        unsigned char y_offset;

        get_ship_position(TRUE, (unsigned char)i, &x_offset, &y_offset);
        draw_picture_at(SHIP_PIRATE, x_offset, y_offset);
    }

    draw_picture_at(ADMIRAL_PORTRAIT, 0, 20);
    print(5, 20, "Our fleet is ambushed by pirates!");
    print(5, 21, "Fight or Run?");

    while (1) {
        unsigned char key;
        key = cgetc();
        switch (key) {
            case 'F':
            case 'f':
                if (visible_friendly_ships > 0) {
                    unsigned char x_offset;
                    unsigned char y_offset;
                    unsigned char ship_index;

                    ship_index = (unsigned char)(rand() % visible_friendly_ships);
                    get_ship_position(FALSE, ship_index, &x_offset, &y_offset);
                    animate_ship_hit(FALSE, x_offset, y_offset);
                }
                if (visible_enemy_ships > 0) {
                    unsigned char x_offset;
                    unsigned char y_offset;
                    unsigned char ship_index;

                    ship_index = (unsigned char)(rand() % visible_enemy_ships);
                    get_ship_position(TRUE, ship_index, &x_offset, &y_offset);
                    animate_ship_hit(TRUE, x_offset, y_offset);
                }
                break;
            case 'R':
            case 'r':
                state.current_screen = SCREEN_DIPLOMACY;
                return;
        }
    }

}
