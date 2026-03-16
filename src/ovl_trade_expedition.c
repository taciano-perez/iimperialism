#include <conio.h>
#include "game.h"

#define PRICE_BOX_Y1 9

void render_trade_market(void) {
    unsigned char nation_index;
    unsigned char i;

    nation_index = get_selected_trade_nation();
    clear_screen();
    print(0, 0, "The nation of");
    print(14, 0, state.foreign_nations[nation_index].name);
    print(27, 0, "Money:");
    print_int_right_aligned(39, 0, state.money);

    render_warehouse_box();

    print(1, PRICE_BOX_Y1-1, "Local market prices");
    box(0, PRICE_BOX_Y1, 39, PRICE_BOX_Y1+4);
    for (i = 0; i < FOREIGN_TRADE_ENTRY_COUNT; ++i) {
        print_int(1, PRICE_BOX_Y1 + 1 + i, i+1);
        print(2, PRICE_BOX_Y1 + 1 + i, ")");
        print(3, PRICE_BOX_Y1 + 1 + i, get_resource_name(state.foreign_nations[nation_index].imports[i]));
        print_int_right_aligned(14, PRICE_BOX_Y1 + 1 + i, state.foreign_nations[nation_index].import_prices[i]);

        print_int(19, PRICE_BOX_Y1 + 1 + i, i+4);
        print(20, PRICE_BOX_Y1 + 1 + i, ")");
        print(21, PRICE_BOX_Y1 + 1 + i, get_resource_name(state.foreign_nations[nation_index].exports[i]));
        print_int_right_aligned(33, PRICE_BOX_Y1 + 1 + i, state.foreign_nations[nation_index].export_prices[i]);
    }
    print(1, PRICE_BOX_Y1+6, "Fleet capacity:");
    print_int_right_aligned(29, PRICE_BOX_Y1+6, state.remaining_turn_capacity);

    draw_picture_at(INDUSTRY_PORTRAIT, 0, 20);
    return;
}
