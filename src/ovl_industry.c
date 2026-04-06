#include "game.h"
#include "overlay.h"

#define BOX_X1 0
#define BOX_Y1 8
#define BOX_X2 39
#define BOX_Y2 13

void render_industry_screen(void) {
    while (1) {
        char key;

        clear_screen();

        print(0, 0, "Ministry of Industry");
        render_turn_funds_header();

        render_warehouse_box();

        /* TRANSPORT & PRODUCTION ORDERS */
        box(BOX_X1, BOX_Y1, BOX_X2, BOX_Y2);
        print_inverted((BOX_X1+1), BOX_Y1, "Transport &");
        print((BOX_X1+1), (BOX_Y1+1), "Timber: ");
        print_int_right_aligned((BOX_X1+11), (BOX_Y1+1), state.transport_timber);
        print((BOX_X1+1), (BOX_Y1+2), "Wool: ");
        print_int_right_aligned((BOX_X1+11), (BOX_Y1+2), state.transport_wool);
        print((BOX_X1+1), (BOX_Y1+3), "Iron: ");
        print_int_right_aligned((BOX_X1+11), (BOX_Y1+3), state.transport_iron);
        print((BOX_X1+1), (BOX_Y1+4), "Coal: ");
        print_int_right_aligned((BOX_X1+11), (BOX_Y1+4), state.transport_coal);

        print_inverted((BOX_X1+13), BOX_Y1, "Production per turn");
        print((BOX_X1+13), (BOX_Y1+1), "Lumber: ");
        print_int_right_aligned((BOX_X1+23), (BOX_Y1+1), state.production_lumber);
        print((BOX_X1+13), (BOX_Y1+2), "Fabric: ");
        print_int_right_aligned((BOX_X1+23), (BOX_Y1+2), state.production_fabric);
        print((BOX_X1+13), (BOX_Y1+3), "Steel: ");
        print_int_right_aligned((BOX_X1+23), (BOX_Y1+3), state.production_steel);
        print((BOX_X1+25), (BOX_Y1+1), "Furniture: ");
        print_int_right_aligned((BOX_X1+38), (BOX_Y1+1), state.production_furniture);
        print((BOX_X1+25), (BOX_Y1+2), "Clothes: ");
        print_int_right_aligned((BOX_X1+38), (BOX_Y1+2), state.production_clothes);
        print((BOX_X1+25), (BOX_Y1+3), "Tools: ");
        print_int_right_aligned((BOX_X1+38), (BOX_Y1+3), state.production_tools);
        print((BOX_X1+25), (BOX_Y1+4), "Guns: ");
        print_int_right_aligned((BOX_X1+38), (BOX_Y1+4), state.production_guns);

        /* ADVISOR */
        draw_picture_at(INDUSTRY_PORTRAIT, 0, 20);
        print(5, 20, "Issue orders for Transport,");
        print(5, 21, "Production, open Ledger, or Quit?");

        key = cgetc_at(38, 21);

        switch (key) {
            case 't':
            case 'T':
                state.current_screen = SCREEN_TRANSPORT;
                return;
            case 'p':
            case 'P':
                state.current_screen = SCREEN_PRODUCTION;
                return;
            case 'l':
            case 'L':
                state.current_screen = SCREEN_LEDGER;
                return;
            case 'q':
            case 'Q':
                state.current_screen = SCREEN_MAIN;
                return;
       }
    }
}
