#include "game.h"

#define BOX_X1 0
#define BOX_Y1 8
#define BOX_X2 39
#define BOX_Y2 14

#define state (*s)

void render_production_screen(GameState *s) {
    clear_screen();
    print(0, 0, "Production Orders");
    render_warehouse_box();

    box(BOX_X1, BOX_Y1, BOX_X2, BOX_Y2);
    print((BOX_X1+1), BOX_Y1, "Production per turn");
    print((BOX_X1+1), (BOX_Y1+1), "Lumber: ");
    print_int_right_aligned((BOX_X1+11), (BOX_Y1+1), state.production_lumber);
    print((BOX_X1+1), (BOX_Y1+2), "Fabric: ");
    print_int_right_aligned((BOX_X1+11), (BOX_Y1+2), state.production_fabric);
    print((BOX_X1+1), (BOX_Y1+3), "Steel: ");
    print_int_right_aligned((BOX_X1+11), (BOX_Y1+3), state.production_steel);
    print((BOX_X1+20), (BOX_Y1+1), "Furniture: ");
    print_int_right_aligned((BOX_X1+34), (BOX_Y1+1), state.production_furniture);
    print((BOX_X1+20), (BOX_Y1+2), "Clothes: ");
    print_int_right_aligned((BOX_X1+34), (BOX_Y1+2), state.production_clothes);
    print((BOX_X1+20), (BOX_Y1+3), "Tools: ");
    print_int_right_aligned((BOX_X1+34), (BOX_Y1+3), state.production_tools);
    print((BOX_X1+20), (BOX_Y1+4), "Guns: ");
    print_int_right_aligned((BOX_X1+34), (BOX_Y1+4), state.production_guns);
    print(1, 13, "Available workers:");
    print_int_right_aligned(24, 13, state.available_workers);

    print(1, 15, "Each unit costs a worker and also:");
    print(1, 16, "Lumber: 2 timber");
    print(1, 17, "Fabric: 2 wool");
    print(1, 18, "Steel:  2 iron");
    print(7, 19, "+ 2 coal");
    print(18, 16, "Furniture: 2 lumber");
    print(18, 17, "Clothes:   2 fabric");
    print(18, 18, "Tools:     2 steel");
    print(18, 19, "Guns:      2 steel");

    draw_picture_at(WISEMAN_PORTRAIT, 0, 20);
    print(5, 20, "What are your orders, sir?");
    print(5, 21, "Change Production per turn, Train");
    print(5, 22, "new workers or Return?");
}
