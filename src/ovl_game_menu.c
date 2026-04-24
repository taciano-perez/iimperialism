#include <conio.h>
#include "game.h"
#include "disk.h"

#define NO_SLOT 255U

const unsigned int game_state_size = sizeof(GameState);
unsigned char save_slot = 0;

#ifndef RWTS_EXPERIMENTAL
static unsigned char save_game(const GameState* game_state, unsigned char slot) {
    save_slot = slot;
    return disk_save_game(game_state);
}

static unsigned char load_game(GameState* game_state, unsigned char slot) {
    save_slot = slot;
    return disk_load_game(game_state);
}

static void show_io_error(unsigned char error_code) {
    clear_input_area();
    print(5, 20, "I/O Error ");
    print_int(15, 20, error_code);
    print(5, 21, "Press any key.");
    cgetc();
}

static unsigned char choose_save_slot(char is_load) {
    DiskSaveSlotInfo info;
    unsigned char i;
    unsigned char error_code;
    char key;

    while (1) {
        clear_screen();
        print_bold(13, 3, is_load ? "Load Game" : "Save Game");

        for (i = 0; i < DISK_SAVE_SLOT_COUNT; ++i) {
            save_slot = i;
            info.valid = 0;
            error_code = disk_read_save_slot_info(&info);
            if (error_code != 0) {
                show_io_error(error_code);
                return NO_SLOT;
            }

            print_int(5, 6 + i, i + 1);
            print(6, 6 + i, ")");
            if (info.valid) {
                print(8, 6 + i, info.nation_name);
                print(25, 6 + i, "Turn ");
                print_int(30, 6 + i, info.turn_number);
            } else {
                print(8, 6 + i, "<Empty>");
            }
        }

        print(6, 14, "1-5) Select");
        print(6, 15, "ESC) Back");

        key = cgetc();
        if (key >= '1' && key <= '5') {
            return (unsigned char)(key - '1');
        }
        if (key == 27) {
            return NO_SLOT;
        }
    }
}
#else
static void show_feature_unavailable(const char* feature_name) {
    clear_input_area();
    print(5, 20, feature_name);
    print(5, 21, "not ready in RWTS build");
    print(5, 22, "Press any key.");
    cgetc();
}
#endif

void render_game_menu_screen(void) {
    while (1) {
        char key;
#ifndef RWTS_EXPERIMENTAL
        unsigned char result;
        unsigned char slot;
#endif

        clear_screen();
        print_bold(13, 3, "IImperialism!");
        box(8, 6, 30, 13);
        print(13, 7, "N) New Game");
        print(13, 8, "L) Load Game");
        print(13, 9, "S) Save Game");
        print(11, 11, "ESC) Resume Game");

        key = cgetc();

        switch (key) {
            case 'N':
            case 'n':
                start_new_game();
                return;

            case 'L':
            case 'l':
#ifdef RWTS_EXPERIMENTAL
                show_feature_unavailable("Load Game");
                break;
#else
                slot = choose_save_slot(1);
                if (slot == NO_SLOT) {
                    break;
                }
                result = load_game(&state, slot);
                if (result != 0) {
                    show_io_error(result);
                }
                return;
#endif

            case 'S':
            case 's':
#ifdef RWTS_EXPERIMENTAL
                show_feature_unavailable("Save Game");
                break;
#else
                slot = choose_save_slot(0);
                if (slot == NO_SLOT) {
                    break;
                }
                result = save_game(&state, slot);
                clear_input_area();
                if (result == 0) {
                    print(5, 20, "Game saved. Press any key.");
                    cgetc();
                } else {
                    show_io_error(result);
                }
                return;
#endif

            case 27: // ESC key
                return;
        }
    }
}
