#include <conio.h>
#include "game.h"

#define state (*s)

void admiralty_build_trader_overlay(GameState *s) {
    unsigned int max_traders;
    unsigned int traders_to_build;

    clear_input_area();
    max_traders = MIN(MIN(state.resources[RESOURCE_LUMBER], state.resources[RESOURCE_FABRIC]), state.available_workers);

    if (max_traders == 0) {
        print(5, 20, "Trader cost: 1 lumber, 1 fabric, &");
        print(5, 21, "1 worker. We lack resources!");
        cgetc();
        return;
    }

    while (1) {
        clear_input_area();
        print(5, 20, "Trader cost: 1 lumber, 1 fabric, &");
        print(5, 21, "1 worker. Build how many? Max:");
        print_int(35, 21, max_traders);
        traders_to_build = scan_uint(5, 22, 5);
        if (traders_to_build > max_traders) {
            print(26, 22, "Too many");
            cgetc();
            continue;
        }

        state.resources[RESOURCE_LUMBER] -= traders_to_build;
        state.resources[RESOURCE_FABRIC] -= traders_to_build;
        state.available_workers -= traders_to_build;
        state.traders += traders_to_build;
        return;
    }
}
