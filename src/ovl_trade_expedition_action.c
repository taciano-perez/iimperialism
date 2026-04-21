#include "game.h"

#define TRADE_MODE_BUY  0
#define TRADE_MODE_SELL 1

static void trade_commodities(unsigned char nation_index, unsigned char mode);

void handle_screen_trade_expedition(void) {
    unsigned char key;
    unsigned char nation_index;

    nation_index = get_selected_trade_nation();

    while (1) {

        clear_area(32, 1, 5, 1);
        print_int_right_aligned_currency(39, 1, state.money);

        clear_area(29, 15, 5, 1);
        print_int_right_aligned(29, 15, state.remaining_turn_capacity);

        clear_input_area();
        print(5, 20, get_diplomacy_string(DSTR_BUY_SELL_QUIT));
        key = (unsigned char)(cgetc_at(23, 20) & 0xDF);
        if (key == 'B') {
            trade_commodities(nation_index, TRADE_MODE_BUY);
            return;
        }
        if (key == 'S') {
            trade_commodities(nation_index, TRADE_MODE_SELL);
            return;
        }
        if (key == 'Q') {
            state.current_screen = SCREEN_DIPLOMACY;
            return;
        }
    }
}

static void trade_commodities(unsigned char nation_index, unsigned char mode) {
    unsigned char i;
    unsigned char key;
    unsigned int quantity;
    unsigned char max_quantity;
    unsigned char resource;
    ForeignNation* nation;
    const unsigned char* trade_list;
    unsigned char menu_base;
    unsigned char price;

    nation = &state.foreign_nations[nation_index];

    if (mode == TRADE_MODE_BUY) {
        trade_list = nation->exports;
        menu_base = 4;
    } else {
        trade_list = nation->imports;
        menu_base = 1;
    }

    while (1) {
        clear_input_area();
        print(5, 20, get_diplomacy_string(mode == TRADE_MODE_BUY ? DSTR_COMMODITY_BUY : DSTR_COMMODITY_SELL));

        for (i = 0; i < FOREIGN_TRADE_ENTRY_COUNT; ++i) {
            print_int((i * 12) + 5, 21, i + menu_base);
            print((i * 12) + 6, 21, ")");
            print((i * 12) + 7, 21, get_resource_name(trade_list[i]));
        }

        while (1) {
            key = cgetc_at(23, 20);
            if (key >= (unsigned char)('0' + menu_base) && key < (unsigned char)('0' + menu_base + FOREIGN_TRADE_ENTRY_COUNT)) {
                i = (unsigned char)(key - '0' - menu_base);
                resource = trade_list[i];
                if (mode == TRADE_MODE_BUY) {
                    price = nation->export_prices[i];
                    max_quantity = MIN(state.remaining_turn_capacity, state.money / price);
                    if ((unsigned int)max_quantity > (MAX_UINT - state.resources[resource])) {
                        max_quantity = (unsigned char)(MAX_UINT - state.resources[resource]);
                    }
                } else {
                    price = nation->import_prices[i];
                    max_quantity = MIN(state.remaining_turn_capacity, state.resources[resource]);
                }

                while (1) {
                    clear_area(28, 22, 3, 1);
                    print(5, 22, get_diplomacy_string(DSTR_HOW_MANY));
                    print_int_right_aligned(28, 22, max_quantity);
                    quantity = scan_uint(31, 22, 3);
                    if (quantity <= max_quantity) {
                        state.remaining_turn_capacity -= quantity;
                        if (mode == TRADE_MODE_BUY) {
                            state.resources[resource] += quantity;
                            state.money -= quantity * price;
                            state.trade_expenses += quantity * price;
                        } else {
                            state.resources[resource] -= quantity;
                            state.money += quantity * price;
                            state.trade_revenue += quantity * price;
                        }
                        // improve trade relations proportionally to the trade * multiplier, but only if not already an ally/colony
                        if (nation->relations != RELATION_ALLY_COLONY) {
                            nation->relations_previous_turn = nation->relations;
                            nation->relations = MIN((unsigned int)nation->relations + (quantity * TRADE_RELATIONS_MULTIPLIER), RELATION_GREAT);
                        }
                        return;
                    }
                }
            }
        }
    }
}
