#include "game.h"
#include "overlay.h"

#define BOX_X1 0
#define BOX_Y1 8
#define BOX_X2 39
#define BOX_Y2 13

static void render_ledger_screen(void) {
    int labor_upkeep = (int)state.available_workers * UPKEEP_COST_PER_WORKER * -1;
    int merchant_upkeep = (int)state.traders * UPKEEP_COST_PER_TRADER * -1;
    int navy_upkeep = ((int)state.warships * UPKEEP_COST_PER_WARSHIP) * -1;
    int balance = (int)state.trade_revenue + (int)state.turn_booty + (int)labor_upkeep
                + (int)merchant_upkeep + (int)navy_upkeep - (int)state.trade_expenses;

    clear_screen();
    print(0, 0, "Ledger Book");
    render_turn_funds_header();

    box(0, 3, 39, 15);
    print(1, 3, "Expenses");
    print(10, 4, "Labor force upkeep");
    print_signed_int_right_aligned_currency(38, 4, labor_upkeep);
    print(10, 5, "Merchant Marine upkeep");
    print_signed_int_right_aligned_currency(38, 5, merchant_upkeep);
    print(10, 6, "Navy upkeep");
    print_signed_int_right_aligned_currency(38, 6, navy_upkeep);
    print(10, 7, "Trade expenses");
    print_signed_int_right_aligned_currency(38, 7, (int)state.trade_expenses * -1);
    print(1, 9, "Income");
    print(10, 10, "Trade revenue");
    print_int_right_aligned_currency(38, 10, state.trade_revenue);
    print(10, 11, "War booty");
    print_int_right_aligned_currency(38, 11, state.turn_booty);
    print(1, 13, "Balance");
    print(10, 14, "Bottom line");
    print_signed_int_right_aligned_currency(38, 14, balance);

    draw_picture_at(INDUSTRY_PORTRAIT, 0, 20);
    print(5, 20, "Quit ledger?");
    cgetc_at(17, 20);
    state.current_screen = SCREEN_INDUSTRY;
}

void render_industry_screen(void) {
    while (1) {
        char key;

        clear_screen();

        print(0, 0, "Ministry of Industry");
        render_turn_funds_header();

        render_warehouse_box();

        /* TRANSPORT & PRODUCTION ORDERS */
        box(BOX_X1, BOX_Y1+1, BOX_X2, BOX_Y2);
        print_inverted((BOX_X1), BOX_Y1, "Transport & ");
        print((BOX_X1+1), (BOX_Y1+1), "Timber: ");
        print_int_right_aligned((BOX_X1+11), (BOX_Y1+1), state.transport_timber);
        print((BOX_X1+1), (BOX_Y1+2), "Wool: ");
        print_int_right_aligned((BOX_X1+11), (BOX_Y1+2), state.transport_wool);
        print((BOX_X1+1), (BOX_Y1+3), "Iron: ");
        print_int_right_aligned((BOX_X1+11), (BOX_Y1+3), state.transport_iron);
        print((BOX_X1+1), (BOX_Y1+4), "Coal: ");
        print_int_right_aligned((BOX_X1+11), (BOX_Y1+4), state.transport_coal);

        print_inverted((BOX_X1+12), BOX_Y1, "Production per turn");
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
                render_ledger_screen();
                return;
            case 'q':
            case 'Q':
                state.current_screen = SCREEN_MAIN;
                return;
       }
    }
}
