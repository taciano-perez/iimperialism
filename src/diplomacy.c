#include "game.h"
#include "overlay.h"

void handle_screen_input_diplomacy(char key) {
    switch (key) {
        case 't':
        case 'T':
            run_overlay(OVL_TRADE_EXPEDITION);
            break;
        case 'r':
        case 'R':
            state.current_screen = SCREEN_INDUSTRY;
            break;
    }
}
