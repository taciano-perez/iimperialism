#include "game.h"

void handle_screen_input_industry(char key) {
    switch (key) {
        case 't':
        case 'T':
            state.current_screen = SCREEN_TRANSPORT;
            break;
        case 'p':
        case 'P':
            state.current_screen = SCREEN_PRODUCTION;
            break;
        case 'a':
        case 'A':
            state.current_screen = SCREEN_ADMIRALTY;
            break;
        case 'e':
        case 'E':
            next_turn();
            break;
    }
}
