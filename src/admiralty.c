#include "game.h"
#include "overlay.h"

void handle_screen_input_admiralty(char key) {
    switch (key) {
        case 't':
        case 'T':
            run_overlay(OVL_ADMIRALTY_TRADER);
            break;
        case 'w':
        case 'W':
            run_overlay(OVL_ADMIRALTY_WARSHIP);
            break;
        case 'q':
        case 'Q':
            state.current_screen = SCREEN_INDUSTRY;
            break;
    }
}
