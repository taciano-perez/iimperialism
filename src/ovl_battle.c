#include <conio.h>
#include <tgi.h>
#include "game.h"

#define MAX_VISIBLE_SHIPS 12

#define state (*s)

static const char STR_BATTLE_FIREPOWER[] = "Firepower:";

static void render_firepower(unsigned char is_enemy, unsigned char ship_count) {
    unsigned char clear_x;
    unsigned char print_x;

    clear_x = is_enemy ? 35 : 12;
    print_x = is_enemy ? 38 : 15;

    clear_area(clear_x, 1, 4, 1);
    print_int_right_aligned(print_x, 1, ship_count * GUNS_PER_FRIGATE);
}

static void get_ship_position(unsigned char is_enemy,
                              unsigned char ship_index,
                              unsigned char* x_offset,
                              unsigned char* y_offset) {
    static const unsigned char ship_x_offsets[MAX_VISIBLE_SHIPS] = {
        0, 6, 12,
        0, 6, 12,
        0, 6, 12,
        0, 6, 12
    };
    static const unsigned char ship_y_offsets[MAX_VISIBLE_SHIPS] = {
        2, 2, 2,
        6, 6, 6,
        10, 10, 10,
        14, 14, 14
    };

    *x_offset = ship_x_offsets[ship_index];
    if (is_enemy) {
        *x_offset += 23;
    }
    *y_offset = ship_y_offsets[ship_index];
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

static void handle_ship_hit(unsigned char is_enemy, unsigned char* ship_count) {
    unsigned char ship_index;
    unsigned char last_index;
    unsigned char x_offset;
    unsigned char y_offset;
    unsigned char last_x_offset;
    unsigned char last_y_offset;
    unsigned char picture;

    if (*ship_count == 0) {
        return;
    }

    ship_index = (unsigned char)rand_range(0, *ship_count - 1);
    get_ship_position(is_enemy, ship_index, &x_offset, &y_offset);

    animate_ship_hit(is_enemy, x_offset, y_offset);

    if (rand_range(0, 1) == 0) {
        picture = is_enemy ? SHIP_PIRATE : SHIP;
        last_index = *ship_count - 1;
        clear_area(x_offset, y_offset, 4, 4);
        --(*ship_count);
        render_firepower(is_enemy, *ship_count);

        if (ship_index != last_index) {
            get_ship_position(is_enemy, last_index, &last_x_offset, &last_y_offset);
            clear_area(last_x_offset, last_y_offset, 4, 4);
            draw_picture_at(picture, x_offset, y_offset);
        }
    }
}

void render_battle_screen(GameState *s) {
    unsigned char i;
    unsigned char visible_friendly_ships;
    unsigned char visible_enemy_ships;

    clear_screen();

    visible_friendly_ships = MIN(state.frigates, MAX_VISIBLE_SHIPS);
    visible_enemy_ships = MIN(state.frigates, MAX_VISIBLE_SHIPS);

    // Render player's navy
    print(0, 0, "Haxaco Navy");
    print(0, 1, STR_BATTLE_FIREPOWER);
    render_firepower(0, visible_friendly_ships);
    for (i = 0; i < visible_friendly_ships; i++) {
        unsigned char x_offset;
        unsigned char y_offset;

        get_ship_position(0, (unsigned char)i, &x_offset, &y_offset);
        draw_picture_at(SHIP, x_offset, y_offset);
    }

    // Render enemy fleet
    print(23, 0, "Enemy Fleet");
    print(23, 1, STR_BATTLE_FIREPOWER);
    render_firepower(1, visible_enemy_ships);
    for (i = 0; i < visible_enemy_ships; i++) {
        unsigned char x_offset;
        unsigned char y_offset;

        get_ship_position(1, (unsigned char)i, &x_offset, &y_offset);
        draw_picture_at(SHIP_PIRATE, x_offset, y_offset);
    }

    draw_picture_at(ADMIRAL_PORTRAIT, 0, 20);
    print(5, 20, "Fleet ambushed by pirates!");
    print(5, 21, "Fight or Run?");

    while (1) {
        unsigned char key;
        key = cgetc();
        switch (key) {
            case 'F':
            case 'f':
                handle_ship_hit(1, &visible_enemy_ships);
                if (visible_enemy_ships == 0) {
                    print(5, 22, "Victory!");
                    state.current_screen = SCREEN_TRADE_EXPEDITION;
                    return;
                }
                handle_ship_hit(0, &visible_friendly_ships);
                if (visible_friendly_ships == 0) {
                    print(5, 22, "Fleet defeated!");
                    state.current_screen = SCREEN_DIPLOMACY;
                    return;
                }
                break;
            case 'R':
            case 'r':
                state.current_screen = SCREEN_DIPLOMACY;
                return;
        }
    }

}
