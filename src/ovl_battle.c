#include <conio.h>
#include <tgi.h>
#include "game.h"

#define MAX_VISIBLE_SHIPS 12


static const char STR_BATTLE_FIREPOWER[] = "Firepower:";
static const char STR_TRADER_LOSS[] = "The buggers sunk a trader!";
static const char STR_FIGHT[] = "Aye, we'll fight!";
static unsigned char current_guns_per_frigate;

static unsigned char get_enemy_fleet_size(void) {
    unsigned char base_ships;
    unsigned char min_ships;
    unsigned char max_ships;

    base_ships = 1U + ((state.turn_number - 1U) / 10U);
    min_ships = (base_ships + 1U) / 2U;
    max_ships = base_ships + (base_ships / 2U);

    if (max_ships < min_ships) {
        max_ships = min_ships;
    }

    return rand_range(min_ships, max_ships);
}

static void render_firepower(unsigned char is_enemy, unsigned char ship_count) {
    unsigned char clear_x;

    clear_x = is_enemy ? 35 : 12;
    clear_area(clear_x, 1, 4, 1);
    print_int_right_aligned(clear_x + 3, 1, ship_count * current_guns_per_frigate);
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
        3, 3, 3,
        7, 7, 7,
        11, 11, 11,
        15, 15, 15
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

static void handle_ship_hit(unsigned char is_enemy,
                            unsigned char* ship_count,
                            unsigned char attacker_ship_count) {
    unsigned char ship_index;
    unsigned char last_index;
    unsigned char x_offset;
    unsigned char y_offset;
    unsigned char last_x_offset;
    unsigned char last_y_offset;
    unsigned char picture;
    unsigned char total_ship_count;

    if (*ship_count == 0) {
        return;
    }

    ship_index = rand_range(0, *ship_count - 1);
    get_ship_position(is_enemy, ship_index, &x_offset, &y_offset);

    animate_ship_hit(is_enemy, x_offset, y_offset);

    total_ship_count = attacker_ship_count + *ship_count;
    if (rand_range(0, total_ship_count - 1) < attacker_ship_count) {
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

static void handle_enemy_trader_hit(void) {
    if (state.traders == 0U) {
        return;
    }

    if (state.traders > 0U && rand_range(1U, 100U) <= BATTLE_TRADER_HIT_CHANCE_PERCENT) {
        print(5, 22, STR_TRADER_LOSS);
        --state.traders;
        clear_area(10, 2, 15, 1);
        print_int_right_aligned(15, 2, state.traders);
        cgetc();
        clear_area(5, 22, 31, 1);
        print(5, 22, STR_FIGHT);
    }
}

void render_battle_screen(void) {
    unsigned char enemy_ships;
    unsigned char i;
    unsigned char visible_friendly_ships;
    unsigned char visible_enemy_ships;

    current_guns_per_frigate = state.guns_per_frigate;
    enemy_ships = get_enemy_fleet_size();

    visible_friendly_ships = MIN(state.frigates, MAX_VISIBLE_SHIPS);
    visible_enemy_ships = MIN(enemy_ships, MAX_VISIBLE_SHIPS);

    // Render player's navy
    print(0, 0, "Haxaco Navy");
    print(0, 1, STR_BATTLE_FIREPOWER);
    print(0, 2, "Traders:");
    render_firepower(0, visible_friendly_ships);
    print_int_right_aligned(15, 2, state.traders);
    for (i = 0; i < visible_friendly_ships; ++i) {
        unsigned char x_offset;
        unsigned char y_offset;

        get_ship_position(0, (unsigned char)i, &x_offset, &y_offset);
        draw_picture_at(SHIP, x_offset, y_offset);
    }

    // Render enemy fleet
    print(23, 0, "Enemy Fleet");
    print(23, 1, STR_BATTLE_FIREPOWER);
    render_firepower(1, visible_enemy_ships);
    for (i = 0; i < visible_enemy_ships; ++i) {
        unsigned char x_offset;
        unsigned char y_offset;

        get_ship_position(1, (unsigned char)i, &x_offset, &y_offset);
        draw_picture_at(SHIP_PIRATE, x_offset, y_offset);
    }

    while (1) {
        unsigned char key;
        unsigned char previous_friendly_ships;
        clear_area(5, 21, 22, 2);
        print(5, 21, "Fight or Run?");
        key = cgetc();
        switch (key) {
            case 'F':
            case 'f':
                print(5, 22, STR_FIGHT);
                for (i = 0; i < visible_friendly_ships; ++i) {
                    handle_ship_hit(1, &visible_enemy_ships, visible_friendly_ships);
                    if (visible_enemy_ships == 0) {
                        clear_input_area();
                        print(5, 20, "Victory!");
                        state.current_screen = SCREEN_TRADE_EXPEDITION;
                        return;
                    }
                    previous_friendly_ships = visible_friendly_ships;
                    handle_ship_hit(0, &visible_friendly_ships, visible_enemy_ships);
                    handle_enemy_trader_hit();
                    if (visible_friendly_ships != previous_friendly_ships) {
                        --state.frigates;
                    }
                    if (visible_friendly_ships == 0) {
                        clear_input_area();
                        print(5, 20, "Defeat!");
                        state.current_screen = SCREEN_DIPLOMACY;
                        return;
                    }
                }
                break;
            case 'R':
            case 'r':
                state.current_screen = SCREEN_DIPLOMACY;
                return;
        }
    }

}
