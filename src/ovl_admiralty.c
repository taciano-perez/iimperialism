#include "game.h"

#define BOX_X1 0
#define BOX_Y1 8
#define BOX_X2 39
#define BOX_Y2 13

char buffer[42];

/* #define state (*s) lets the function body use "state.field" syntax
 * identical to the original main.c code.  Must come AFTER #include "game.h"
 * so the extern declaration there is processed without the macro active. */
#define state (*s)

void render_admiralty_screen(GameState *s) {
    clear_screen();

    /* Header: "President <name> of <nation>  12 FEB 2026" */
    print(0,  0, "Admiralty Office ");
    print(28, 0, "12 FEB 2026");

    render_warehouse_box();

    /* TRANSPORT & PRODUCTION ORDERS */
    box(BOX_X1, BOX_Y1, BOX_X2, BOX_Y2);
    print((BOX_X1+1), BOX_Y1, "Merchant &");
    print((BOX_X1+1), (BOX_Y1+1), "Trader: ");
    print_int_right_aligned((BOX_X1+11), (BOX_Y1+1), state.transport_timber);

    print((BOX_X1+13), BOX_Y1, "Warships");
    print((BOX_X1+13), (BOX_Y1+1), "Frigate: ");
    print_int_right_aligned((BOX_X1+23), (BOX_Y1+1), state.production_lumber);

    /* ADVISOR */
    draw_picture_at(ADMIRAL_PORTRAIT, 0, 20);
    print(5, 20, "Awaiting orders, sir.");
    print(5, 21, "Build ship, ");
    print(5, 22, "or Return?");
}
