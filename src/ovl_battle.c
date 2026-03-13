#include "game.h"

#define state (*s)

void render_battle_screen(GameState *s) {
    unsigned int i;

    clear_screen();

    // Render player's navy
    print(0, 0, "Haxaco Navy");
    print(0, 1, "Firepower:");
    print_int_right_aligned(15, 1, state.frigates * GUNS_PER_FRIGATE);
    for (i = 0; i < MIN(state.frigates, 8); i++) {
        if (i % 2 == 0) {
            draw_picture_at(SHIP, 0, (2 + i * 2));
        } else {
            draw_picture_at(SHIP, 6, (i * 2));
        }
    }

    // Render pirate fleet
    print(24, 0, "Pirate Fleet");
    print(24, 1, "Firepower:");
    print_int_right_aligned(39, 1, state.frigates * GUNS_PER_FRIGATE);
    for (i = 0; i < MIN(state.frigates, 8); i++) {
        if (i % 2 == 0) {
            draw_picture_at(SHIP_PIRATE, 29, (2 + i * 2));
        } else {
            draw_picture_at(SHIP_PIRATE, 35, (i * 2));
        }
    }

    draw_picture_at(ADMIRAL_PORTRAIT, 0, 20);
    print(5, 20, "Our fleet is ambushed by pirates!");
    print(5, 21, "Fight or Run?");

}
