#include "game.h"

#define state (*s)

void render_battle_screen(GameState *s) {
    clear_screen();
    print(24, 0, "Firepower:");
    print_int_right_aligned(39, 0, state.frigates * GUNS_PER_FRIGATE);

    draw_picture_at(SHIP, 0, 2);

    draw_picture_at(ADMIRAL_PORTRAIT, 0, 20);
    print(5, 20, "Our fleet is ambushed by pirates!");
    print(5, 21, "Fight or Run?");

}
