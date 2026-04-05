#include "game.h"

void render_ledger_screen(void) {
    int labor_upkeep = (int)state.available_workers * UPKEEP_COST_PER_WORKER * -1;
    int merchant_upkeep = (int)state.traders * UPKEEP_COST_PER_TRADER * -1;
    int navy_upkeep = ((int)state.frigates * UPKEEP_COST_PER_WARSHIP) * -1;
    int balance = (int)state.trade_revenue + (int)state.turn_booty + (int) labor_upkeep + (int)merchant_upkeep + (int)navy_upkeep - (int)state.trade_expenses;

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
    print_signed_int_right_aligned_currency(38, 7, (int)state.trade_expenses*-1);
    print(1, 9, "Income");
    print(10, 10, "Trade revenue");
    print_int_right_aligned_currency(38, 10, state.trade_revenue);
    print(10, 11, "War booty");
    print_int_right_aligned_currency(38, 11, state.turn_booty);
    print(1, 13, "Balance");
    print(10, 14, "Bottom line");
    print_signed_int_right_aligned_currency(38, 14, balance);

    /* ADVISOR */
    draw_picture_at(INDUSTRY_PORTRAIT, 0, 20);
    print(5, 20, "Any key to close ledger.");
    cgetc_at(29, 20);
    state.current_screen = SCREEN_INDUSTRY;
}