#include <conio.h>
#include "game.h"

/* Low-level ProDOS helper linked into this overlay. */
extern unsigned char __fastcall__ prodos_save_game(const GameState* state);
extern unsigned char __fastcall__ prodos_load_game(GameState* state);
const unsigned int game_state_size = sizeof(GameState);

static int save_game(const GameState* game_state) {
    return prodos_save_game(game_state) == 0 ? 0 : -1;
}

static int load_game(GameState* game_state) {
    return prodos_load_game(game_state) == 0 ? 0 : -1;
}

void render_game_menu_screen(void) {
    while (1) {
        char key;
        int result;

        clear_screen();
        print(12, 2, "Game Menu");
        box(8, 4, 31, 14);
        print(11, 6, "N) New Game");
        print(11, 8, "L) Load Game");
        print(11, 10, "S) Save Game");
        print(11, 12, "ESC returns");

        key = cgetc();

        switch (key) {
            case 'N':
            case 'n':
                init_game();
                state.current_screen = SCREEN_INDUSTRY;
                return;

            case 'L':
            case 'l':
                result = load_game(&state);
                clear_input_area();
                if (result != 0) {
                    print(5, 20, "Load failed.");
                }
                return;

            case 'S':
            case 's':
                result = save_game(&state);
                clear_input_area();
                if (result == 0) {
                    print(5, 20, "Game saved. Press any key.");
                } else {
                    print(5, 20, "Save failed.");
                }
                cgetc();
                return;

            case 27: // ESC key
                return;
        }
    }
}
