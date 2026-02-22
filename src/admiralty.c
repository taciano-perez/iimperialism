#include <conio.h>
#include <stdio.h>
#include "game.h"

static char buffer[42];

static void build_trader(void) {
    unsigned int max_traders;
    unsigned int traders_to_build;

    clear_input_area();
    max_traders = MIN(MIN(state.lumber, state.fabric), (state.available_workers));
    if (max_traders == 0) {
        print(5, 20, "Sir, a trader costs 1 lumber, 1 fabric,");
        print(5, 21, "and 1 worker. We lack resources!");
        cgetc();
        return;
    } else {
        while (1) {
            print(5, 20, "A Trader costs 1 lumber, 1 fabric,");
            print(5, 21, "and 1 worker. Build how many?");
            sprintf(buffer, "(max %u)", max_traders);
            print (5, 22, buffer);
            traders_to_build = scan_uint(12, 22, 5);
            if (traders_to_build > max_traders) {
                print(19, 22, "Not enough resources!");
                cgetc();
                clear_input_area();
                continue;
            } else {
                state.lumber -= traders_to_build;
                state.fabric -= traders_to_build;
                state.available_workers -= traders_to_build;
                state.traders += traders_to_build;
                break;
            }
        }
    }
}

void handle_screen_input_admiralty(char key) {
        switch (key) {
        case 't':
        case 'T':
            build_trader();
            break;
        case 'w':
        case 'W':
            //build_warship();
            break;
        case 'r':
        case 'R':
            state.current_screen = SCREEN_INDUSTRY;
            break;
    }

}