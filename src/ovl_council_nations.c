#include <conio.h>
#include "game.h"

static const char STR_TITLE[] = "Council of Nations";
static const char STR_PROMPT[] = "Press any key to continue";

void render_council_nations_screen(void) {
    clear_screen();
    print(11, 0, STR_TITLE);

    print(0, 4, "Loke");
    draw_picture_at(COUNTRY1, 5, 4);
    
    draw_picture_at(COUNTRY2, 10, 6);
    print(14, 9, "Kathay");

    print(36, 1, "N");
    print(34, 3, "W");
    draw_picture_at(COMPASS, 34, 2);
    print(38, 3, "E");
    print(36, 5, "S");


    print(9, 22, STR_PROMPT);
    cgetc();
    state.current_screen = SCREEN_INDUSTRY;
}
