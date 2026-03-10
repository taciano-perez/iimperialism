#include <conio.h>
#include "game.h"

#define PRICE_BOX_Y1 9

#define state (*s)

static void buy_commodities(GameState *s, unsigned char nation_index);

void render_trade_market(GameState *s) {
    unsigned char nation_index;
    unsigned char i;
    unsigned char key;

    nation_index = get_selected_trade_nation();
    clear_screen();
    print(0, 0, "The nation of");
    print(14, 0, state.foreign_nations[nation_index].name);

    render_warehouse_box();

    print(1, PRICE_BOX_Y1-1, "Local market prices");
    box(0, PRICE_BOX_Y1, 39, PRICE_BOX_Y1+4);
    for (i = 0; i < FOREIGN_TRADE_ENTRY_COUNT; ++i) {
        print_int(0, PRICE_BOX_Y1 + 1 + i, i+1);
        print(1, PRICE_BOX_Y1 + 1 + i, ")");
        print(2, PRICE_BOX_Y1 + 1 + i, get_resource_name(state.foreign_nations[nation_index].imports[i]));
        print_int_right_aligned(14, PRICE_BOX_Y1 + 1 + i, state.foreign_nations[nation_index].import_prices[i]);

        print_int(19, PRICE_BOX_Y1 + 1 + i, i+4);
        print(20, PRICE_BOX_Y1 + 1 + i, ")");
        print(21, PRICE_BOX_Y1 + 1 + i, get_resource_name(state.foreign_nations[nation_index].exports[i]));
        print_int_right_aligned(33, PRICE_BOX_Y1 + 1 + i, state.foreign_nations[nation_index].export_prices[i]);
    }
    print(1, PRICE_BOX_Y1+6, "Fleet capacity:");
    print_int_right_aligned(29, PRICE_BOX_Y1+6, state.traders * CAPACITY_PER_TRADER);

    draw_picture_at(INDUSTRY_PORTRAIT, 0, 20);
    while (1) {
        clear_input_area();
        print(5, 20, "Buy, Sell or Quit?");
        key = cgetc();
        switch (key) {
            case 'B':
            case 'b':
                buy_commodities(s, nation_index);
                break;
            case 'Q':
            case 'q':
                state.current_screen = SCREEN_DIPLOMACY;
                return;
        }
    }
}

static void buy_commodities(GameState *s, unsigned char nation_index) {
    unsigned char i;
    unsigned int input;

    while (1) {
        clear_input_area();
        print(5, 20, "Commodity to buy?");
        for (i = 0; i < FOREIGN_TRADE_ENTRY_COUNT; ++i) {
            print_int((i*9)+5, 21, i+4);
            print((i*9)+6, 21, ")");
            print((i*9)+7, 21, get_resource_name(state.foreign_nations[nation_index].exports[i]));
        }
        while (1) {
            input = scan_uint(35, 21, 1);
            switch (input) {
                case 4:
                case 5:
                case 6:
                    print(5, 22, get_resource_name(state.foreign_nations[nation_index].exports[input-4]));
                    break;
            }
        }   
    }
}
