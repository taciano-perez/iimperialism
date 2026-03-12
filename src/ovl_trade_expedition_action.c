#include <conio.h>
#include "game.h"

#define state (*s)

static void buy_commodities(GameState *s, unsigned char nation_index, unsigned int capacity);

void handle_screen_trade_expedition(GameState *s) {
    unsigned char key;
    unsigned char nation_index;

    nation_index = get_selected_trade_nation();

    while (1) {
        clear_area(29, 15, 5, 1);
        print_int_right_aligned(29, 15, state.remaining_turn_capacity);

        clear_input_area();
        print(5, 20, "Buy, Sell or Quit?");
        key = cgetc();
        switch (key) {
            case 'B':
            case 'b':
                buy_commodities(s, nation_index, state.remaining_turn_capacity);
                return;
            case 'Q':
            case 'q':
                state.current_screen = SCREEN_DIPLOMACY;
                return;
        }
    }
}

static void buy_commodities(GameState *s, unsigned char nation_index, unsigned int capacity) {
    unsigned char i;
    unsigned int input;
    unsigned int quantity;
    unsigned char selected_export_index;

    while (1) {
        clear_input_area();
        print(5, 20, "Commodity to buy?");
        for (i = 0; i < FOREIGN_TRADE_ENTRY_COUNT; ++i) {
            print_int((i * 12) + 5, 21, i + 4);
            print((i * 12) + 6, 21, ")");
            print((i * 12) + 7, 21, get_resource_name(state.foreign_nations[nation_index].exports[i]));
        }
        while (1) {
            input = scan_uint(38, 21, 1);
            switch (input) {
                case 4:
                case 5:
                case 6:
                    selected_export_index = (unsigned char)(input - 4);
                    while (1) {
                        clear_area(28, 22, 3, 1);
                        print(5, 22, "How many? (Max:    )");
                        print_int_right_aligned(23, 22, capacity);
                        quantity = scan_uint(28, 22, 3);
                        if (quantity <= capacity) {
                            capacity -= quantity;
                            state.remaining_turn_capacity = capacity;
                            state.resources[state.foreign_nations[nation_index].exports[selected_export_index]] += quantity;
                            return;
                        }
                    }
            }
        }
    }
}
