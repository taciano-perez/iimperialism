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

void render_industry_screen(GameState *s) {
    clear_screen();

    /* Header: "President <name> of <nation>  12 FEB 2026" */
    print(0,  0, "President ");
    print(10, 0, state.player_name);
    print(18, 0, "of");
    print(21, 0, state.nation_name);
    print(28, 0, "12 FEB 2026");

    render_warehouse_box();

    /* TRANSPORT & PRODUCTION ORDERS */
    box(BOX_X1, BOX_Y1, BOX_X2, BOX_Y2);
    print((BOX_X1+1), BOX_Y1, "Transport &");
    print((BOX_X1+1), (BOX_Y1+1), "Timber: ");
    print_int_right_aligned((BOX_X1+11), (BOX_Y1+1), state.transport_timber);
    print((BOX_X1+1), (BOX_Y1+2), "Wool: ");
    print_int_right_aligned((BOX_X1+11), (BOX_Y1+2), state.transport_wool);
    print((BOX_X1+1), (BOX_Y1+3), "Iron: ");
    print_int_right_aligned((BOX_X1+11), (BOX_Y1+3), state.transport_iron);
    print((BOX_X1+1), (BOX_Y1+4), "Coal: ");
    print_int_right_aligned((BOX_X1+11), (BOX_Y1+4), state.transport_coal);

    print((BOX_X1+13), BOX_Y1, "Production per turn");
    print((BOX_X1+13), (BOX_Y1+1), "Lumber: ");
    print_int_right_aligned((BOX_X1+23), (BOX_Y1+1), state.production_lumber);
    print((BOX_X1+13), (BOX_Y1+2), "Fabric: ");
    print_int_right_aligned((BOX_X1+23), (BOX_Y1+2), state.production_fabric);
    print((BOX_X1+13), (BOX_Y1+3), "Steel: ");
    print_int_right_aligned((BOX_X1+23), (BOX_Y1+3), state.production_steel);
    print((BOX_X1+25), (BOX_Y1+1), "Furniture: ");
    print_int_right_aligned((BOX_X1+39), (BOX_Y1+1), state.production_furniture);
    print((BOX_X1+25), (BOX_Y1+2), "Clothes: ");
    print_int_right_aligned((BOX_X1+39), (BOX_Y1+2), state.production_clothes);
    print((BOX_X1+25), (BOX_Y1+3), "Tools: ");
    print_int_right_aligned((BOX_X1+39), (BOX_Y1+3), state.production_tools);
    print((BOX_X1+25), (BOX_Y1+4), "Cannons: ");
    print_int_right_aligned((BOX_X1+39), (BOX_Y1+4), state.production_cannons);

    /* ADVISOR */
    draw_picture_at(WISEMAN_PORTRAIT, 0, 20);
    print(5, 20, "What are your orders, sir?");
    print(5, 21, "Change Transport, Production, ");
    print(5, 22, "or End turn?");
}
