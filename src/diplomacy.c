#include "game.h"

void handle_screen_input_diplomacy(char key) {
    switch (key) {
        case 'r':
        case 'R':
            state.current_screen = SCREEN_INDUSTRY;
            break;
    }
}
