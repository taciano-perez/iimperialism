#include <conio.h>
#include <stdio.h>
#include "game.h"
#include "strings.h"

static char buffer[42];

static void build_trader(void) {
    unsigned int max_traders;
    unsigned int traders_to_build;

    clear_input_area();
    max_traders = MIN(MIN(state.lumber, state.fabric), (state.available_workers));
    if (max_traders == 0) {
        print(5, 20, STR_TRADER_COST);
        print(5, 21, "and 1 worker. We lack resources!");
        cgetc();
        return;
    } else {
        while (1) {
            print(5, 20, STR_TRADER_COST);
            print(5, 21, "and 1 worker. Build how many?");
            sprintf(buffer, STR_MAX_FMT, max_traders);
            print (5, 22, buffer);
            traders_to_build = scan_uint(12, 22, 5);
            if (traders_to_build > max_traders) {
                print(19, 22, STR_NOT_ENOUGH_RESOURCES);
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

#pragma code-name (push, "LOWCODE")

static void build_warship(void) {
    unsigned int max_warships;
    unsigned int warships_to_build;

    clear_input_area();
    // max_warships = 2;
    max_warships = MIN(MIN(MIN(state.lumber, state.fabric), (state.guns)), (state.available_workers));
    if (max_warships == 0) {
        print(5, 20, STR_WARSHIP_COST);
        print(5, 21, STR_WARSHIP_COST2);
        print(25, 22, STR_NOT_ENOUGH_RESOURCES);
        cgetc();
        return;
    } else {
        while (1) {
            print(5, 20, STR_WARSHIP_COST);
            print(5, 21, STR_WARSHIP_COST2);
            print(25, 21, "Build how many?");
            sprintf(buffer, STR_MAX_FMT, max_warships);
            print (5, 22, buffer);
            warships_to_build = scan_uint(12, 22, 5);
            if (warships_to_build > max_warships) {
                print(1, 22, STR_SIR_WE_LACK_RESOURCES);
                cgetc();
                clear_input_area();
                continue;
            } else {
                state.lumber -= warships_to_build;
                state.fabric -= warships_to_build;
                state.guns -= warships_to_build;
                state.available_workers -= warships_to_build;
                state.frigates += warships_to_build;
                break;
            }
        }
    }
}

#pragma code-name (pop)

void handle_screen_input_admiralty(char key) {
        switch (key) {
        case 't':
        case 'T':
            build_trader();
            break;
        case 'w':
        case 'W':
            build_warship();
            break;
        case 'r':
        case 'R':
            state.current_screen = SCREEN_INDUSTRY;
            break;
    }

}
