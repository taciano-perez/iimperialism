#include "game.h"

#define state (*s)

void render_battle_screen(GameState *s) {
    unsigned int i;
    unsigned char player_ship_columns[3] = { 0, 6, 12 };
    unsigned char pirate_ship_columns[3] = { 23, 29, 35 };

    clear_screen();

    // Render player's navy
    print(0, 0, "Haxaco Navy");
    print(0, 1, "Firepower:");
    print_int_right_aligned(15, 1, state.frigates * GUNS_PER_FRIGATE);
    for (i = 0; i < MIN(state.frigates, 12); i++) {
        draw_picture_at(SHIP, player_ship_columns[i % 3], 2 + ((i / 3) * 4));
    }

    // Render pirate fleet
    print(23, 0, "Pirate Fleet");
    print(23, 1, "Firepower:");
    print_int_right_aligned(38, 1, state.frigates * GUNS_PER_FRIGATE);
    for (i = 0; i < MIN(state.frigates, 12); i++) {
        draw_picture_at(SHIP_PIRATE, pirate_ship_columns[i % 3], 2 + ((i / 3) * 4));
    }

    draw_picture_at(ADMIRAL_PORTRAIT, 0, 20);
    print(5, 20, "Our fleet is ambushed by pirates!");
    print(5, 21, "Fight or Run?");

}
