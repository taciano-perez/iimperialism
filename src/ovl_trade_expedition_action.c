#include <conio.h>
#include "game.h"

#define state (*s)

#define TRADE_MODE_BUY  0
#define TRADE_MODE_SELL 1

static void trade_commodities(GameState *s, unsigned char nation_index, unsigned char mode);

void handle_screen_trade_expedition(GameState *s) {
    unsigned char key;
    unsigned char nation_index;

    nation_index = get_selected_trade_nation();

    while (1) {

        clear_area(32, 0, 5, 1);
        print_int_right_aligned(39, 0, state.money);

        clear_area(29, 15, 5, 1);
        print_int_right_aligned(29, 15, state.remaining_turn_capacity);

        clear_input_area();
        print(5, 20, "Buy, Sell or Quit?");
        key = cgetc();
        switch (key) {
            case 'B':
            case 'b':
                trade_commodities(s, nation_index, TRADE_MODE_BUY);
                return;
            case 'S':
            case 's':
                trade_commodities(s, nation_index, TRADE_MODE_SELL);
                return;
            case 'Q':
            case 'q':
                state.current_screen = SCREEN_DIPLOMACY;
                return;
        }
    }
}

static void trade_commodities(GameState *s, unsigned char nation_index, unsigned char mode) {
    unsigned char i;
    unsigned int input;
    unsigned int quantity;
    unsigned int max_quantity;
    unsigned char resource;
    const unsigned char* trade_list;
    unsigned char menu_base;
    unsigned int price;

    if (mode == TRADE_MODE_BUY) {
        trade_list = state.foreign_nations[nation_index].exports;
        menu_base = 4;
    } else {
        trade_list = state.foreign_nations[nation_index].imports;
        menu_base = 1;
    }

    while (1) {
        clear_input_area();
        if (mode == TRADE_MODE_BUY) {
            print(5, 20, "Commodity to buy?");
        } else {
            print(5, 20, "Commodity to sell?");
        }

        for (i = 0; i < FOREIGN_TRADE_ENTRY_COUNT; ++i) {
            print_int((i * 12) + 5, 21, i + menu_base);
            print((i * 12) + 6, 21, ")");
            print((i * 12) + 7, 21, get_resource_name(trade_list[i]));
        }

        while (1) {
            input = scan_uint(39, 21, 1);
            if (input >= menu_base && input < (unsigned int)(menu_base + FOREIGN_TRADE_ENTRY_COUNT)) {
                i = (unsigned char)(input - menu_base);
                resource = trade_list[i];
                if (mode == TRADE_MODE_BUY) {
                    price = state.foreign_nations[nation_index].export_prices[i];
                } else {
                    price = state.foreign_nations[nation_index].import_prices[i];
                }
                max_quantity = get_trade_max_quantity(mode, resource, price);

                while (1) {
                    clear_area(28, 22, 3, 1);
                    print(5, 22, "How many? (Max:    )");
                    print_int_right_aligned(23, 22, max_quantity);
                    quantity = scan_uint(28, 22, 3);
                    if (quantity <= max_quantity) {
                        apply_trade(mode, resource, quantity, price);
                        return;
                    }
                }
            }
        }
    }
}
