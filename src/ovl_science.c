#include <conio.h>
#include "game.h"

#define state (*s)

static const char STR_WAGON_CAPACITY[] = "x wagon capacity";
static const char STR_WARSHIP_FIREPOWER[] = "x warship firepower";

void render_science_screen(GameState *s) {
    clear_screen();
    print(0, 0, "Patent Office");
    print(27, 0, "Money:");
    print_int_right_aligned(39, 0, state.money);

    print(1, 3, "Research");
    print(35, 3, "Cost");
    box(0, 4, 39, 16);

    print(1, 4, "1)Railways");
    print_int_right_aligned(38, 4, 500);
    print(15, 5, "2");
    print(16, 5, STR_WAGON_CAPACITY);
    print(1, 6, "2)Flush decks");
    print_int_right_aligned(38, 6, 1000);
    print(15, 7, "2");
    print(16, 7, STR_WARSHIP_FIREPOWER);
    print(1, 8, "3)Steel rails");
    print_int_right_aligned(38, 8, 2000);
    print(15, 9, "3");
    print(16, 9, STR_WAGON_CAPACITY);
    print(1, 10, "4)Shell guns");
    print_int_right_aligned(38, 10, 4000);
    print(15, 11, "3");
    print(16, 11, STR_WARSHIP_FIREPOWER);
    print(1, 12, "5)Telegraph");
    print_int_right_aligned(38, 12, 6000);
    print(15, 13, "4");
    print(16, 13, STR_WAGON_CAPACITY);
    print(1, 14, "6)Breech loaders");
    print_int_right_aligned(38, 14, 8000);
    print(15, 15, "4");
    print(16, 15, STR_WARSHIP_FIREPOWER);

    /* ADVISOR */
    draw_picture_at(SCIENCE_PORTRAIT, 0, 20);
    print(5, 20, "Invest in Research or Quit?");
    while (1) {
        unsigned char key;
        key = cgetc();
        switch (key) {
            case 'R':
            case 'r':
                while (1) {
                    print (5, 21, "Select research (1-6)");
                    cgetc();
                    return;
                    /* TODO: handle research selection */
                }
                break;
            case 'Q':
            case 'q':
                state.current_screen = SCREEN_INDUSTRY;
                return;
        }
    }
}
