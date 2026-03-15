#include <conio.h>
#include "game.h"

#define state (*s)

void render_science_screen(GameState *s) {
    clear_screen();
    print(0, 0, "Science Office");
    print(0, 2, "Testing science overlay.");
    print(0, 4, "Press any key for Industry.");
    cgetc();
    state.current_screen = SCREEN_INDUSTRY;
}
