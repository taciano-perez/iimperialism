#include <conio.h>
#include "game.h"

#define PRICE_BOX_Y1 9

#define state (*s)

static void render_trade_screen(GameState *s, unsigned char nation_index);

void trade_expedition(GameState *s) {
    unsigned int selection;
    unsigned char nation_index;

    while (1) {
        clear_input_area();
        print(5, 20, "Trade with which nation (1-5)?");

        selection = scan_uint(36, 20, 1);
        if (selection < 1 || selection > FOREIGN_NATION_COUNT) {
            print(5, 21, "Invalid nation");
            cgetc();
            continue;
        }
        nation_index = (unsigned char)(selection - 1);

        print(5, 22, "Fleet sailing to the Sea of");
        print(33, 22, state.foreign_nations[nation_index].name);
        cgetc();
        render_trade_screen(s, nation_index);
        return;
    }
}

static void render_trade_screen(GameState *s, unsigned char nation_index) {
    unsigned char i;


    clear_screen();
    print(0, 0, "The nation of");
    print(14, 0, state.foreign_nations[nation_index].name);

    render_warehouse_box();

    print(1, PRICE_BOX_Y1-1, "Market of");
    print(11, PRICE_BOX_Y1-1, state.foreign_nations[nation_index].name);
    box(0, PRICE_BOX_Y1, 39, PRICE_BOX_Y1+4);
    for (i = 0; i < FOREIGN_TRADE_ENTRY_COUNT; ++i) {
        print(1, PRICE_BOX_Y1 + 1 + i, get_resource_name(state.foreign_nations[nation_index].imports[i]));
        print(20, PRICE_BOX_Y1 + 1 + i, get_resource_name(state.foreign_nations[nation_index].exports[i]));
    }

    cgetc();
    return;
}
