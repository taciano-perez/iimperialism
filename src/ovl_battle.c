#include <conio.h>
#include <tgi.h>
#include "game.h"
#include "sound.h"

#define MAX_VISIBLE_SHIPS 12

static const char STR_BATTLE_FIREPOWER[] = "Firepower:";
static const char STR_TRADER_LOSS[] = "The buggers sunk a trader!";
static const char STR_FIGHT[] = "Aye, we'll fight!";
static const char STR_BOUNTY[] = "We captured a booty of $";

static void render_firepower(unsigned char is_enemy, unsigned char ship_count) {
    unsigned char clear_x;

    clear_x = is_enemy ? 35 : 12;
    clear_area(clear_x, 1, 4, 1);
    print_int_right_aligned(clear_x + 3, 1, ship_count * state.guns_per_frigate);
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
    unsigned char i;

    if (*ship_count == 0) {
        return;
    }

    ship_index = rand_range(0, *ship_count - 1);
    get_ship_position(is_enemy, ship_index, &x_offset, &y_offset);

    picture = is_enemy ? SHIP_PIRATE : SHIP;
    for (i = 0; i < 6; ++i) {
        paint_area(x_offset, y_offset, 4, 3, TGI_COLOR_WHITE);
        draw_picture_at(picture, x_offset, y_offset);
    }

    total_ship_count = attacker_ship_count + *ship_count;
    if (rand_range(0, total_ship_count - 1) < attacker_ship_count) {
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

static void trader_lost() {
    print(5, 22, STR_TRADER_LOSS);
    --state.traders;
    state.remaining_turn_capacity = state.traders * CAPACITY_PER_TRADER_BASE;
    clear_area(10, 2, 15, 1);
    print_int_right_aligned(15, 2, state.traders);
    play_sound(SOUND_MUSIC_SCALE);
    wait_three_seconds_or_keypress();
}

void render_battle_screen(void) {
    unsigned char base_ships;
    unsigned char enemy_ships;
    unsigned char i;
    unsigned int bounty;
    unsigned char modifier_percent;
    unsigned char min_ships;
    unsigned char max_ships;
    unsigned char visible_friendly_ships;
    unsigned char visible_enemy_ships;
    
    base_ships = 1U + ((state.turn_number - 1U) / 10U);
    min_ships = (base_ships + 1U) / 2U;
    max_ships = base_ships + (base_ships / 2U);
    enemy_ships = rand_range(min_ships, max_ships);

    visible_friendly_ships = MIN(state.frigates, MAX_VISIBLE_SHIPS);
    visible_enemy_ships = MIN(enemy_ships, MAX_VISIBLE_SHIPS);

    // Render player's navy
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
        key = cgetc_at(18, 21);
        switch (key) {
            case 'F':
            case 'f':
                print(5, 22, STR_FIGHT);
                for (i = 0; i < visible_friendly_ships; ++i) {
                    handle_ship_hit(1, &visible_enemy_ships, visible_friendly_ships);
                    if (visible_enemy_ships == 0) {
                        modifier_percent = rand_range(100U - BATTLE_BOUNTY_VARIANCE_PERCENT,
                                                      100U + BATTLE_BOUNTY_VARIANCE_PERCENT);
                        bounty = ((unsigned int)enemy_ships * 10U * modifier_percent) / 100U;
                        state.money += bounty;
                        clear_input_area();
                        print(5, 20, "Victory!");
                        print(5, 21, STR_BOUNTY);
                        print_int(29, 21, bounty);
                        state.current_screen = SCREEN_TRADE_EXPEDITION;
                        play_sound_alert();
                        wait_three_seconds_or_keypress();
                        return;
                    }
                    previous_friendly_ships = visible_friendly_ships;
                    handle_ship_hit(0, &visible_friendly_ships, visible_enemy_ships);
                    if (state.traders != 0U && rand_range(1U, 100U) <= BATTLE_TRADER_HIT_CHANCE_PERCENT) {
                        trader_lost();
                        clear_area(5, 22, 31, 1);
                        print(5, 22, STR_FIGHT);
                    }
                    if (visible_friendly_ships != previous_friendly_ships) {
                        --state.frigates;
                    }
                    if (visible_friendly_ships == 0) {
                        clear_input_area();
                        print(5, 20, "Defeat!");
                        state.current_screen = SCREEN_DIPLOMACY;
                        play_sound_alert();
                        wait_three_seconds_or_keypress();
                        return;
                    }
                }
                break;
            case 'R':
            case 'r':
                if (state.traders != 0U && rand_range(1U, 100U) <= BATTLE_RUN_TRADER_HIT_CHANCE_PERCENT) {
                    trader_lost();
                }
                state.current_screen = SCREEN_DIPLOMACY;
                return;
        }
    }

}
