#include <conio.h>
#include "game.h"

#define state (*s)

void admiralty_build_warship_overlay(GameState *s) {
    unsigned int max_warships;
    unsigned int warships_to_build;

    clear_input_area();
    max_warships = MIN(MIN(MIN(state.lumber, state.fabric), state.guns),
                       state.available_workers);

    if (max_warships == 0) {
        print(5, 20, "Frigate cost: 1 lumber, 1 fabric,");
        print(5, 21, "1 gun, 1 worker. Not enough.");
        cgetc();
        return;
    }

    while (1) {
        clear_input_area();
        print(5, 20, "Frigate cost: 1 lumber, 1 fabric,");
        print(5, 21, "1 gun, 1 worker. Build how many?");
        print(5, 22, "Max:");
        print_int_right_aligned(14, 22, max_warships);

        warships_to_build = scan_uint(20, 22, 5);
        if (warships_to_build > max_warships) {
            print(22, 22, "Lack resources");
            cgetc();
            continue;
        }

        state.lumber -= warships_to_build;
        state.fabric -= warships_to_build;
        state.guns -= warships_to_build;
        state.available_workers -= warships_to_build;
        state.frigates += warships_to_build;
        return;
    }
}
