#include "game.h"
#include "strings.h"

#define BOX_X1 0
#define BOX_Y1 2
#define BOX_X2 39
#define BOX_Y2 19

#define NATION_X 1
#define RELATION_X 10
#define EXPORT_X 20
#define IMPORT_X 30
#define FIRST_ROW_Y 4
#define ROW_HEIGHT 3

#define state (*s)

static unsigned char get_row_y(unsigned char nation_index);
static void render_trade_column(unsigned char x, unsigned char y, const unsigned int* resources);
static void render_nation_row(GameState* s, unsigned char nation_index);

void render_diplomacy_screen(GameState *s) {
    unsigned char i;

    clear_screen();
    print(0, 0, "Foreign Office");
    print(34, 0, "Turn:");
    print_int_right_aligned(39, 0, state.turn_number);

    print(NATION_X, 2, "Nation");
    print(RELATION_X, 2, "Relations");
    print(EXPORT_X, 2, "Exports");
    print(IMPORT_X, 2, "Imports");

    for (i = 0; i < FOREIGN_NATION_COUNT; ++i) {
        render_nation_row(s, i);
    }

    draw_picture_at(WISEMAN_PORTRAIT, 0, 20);
    print(5, 20, "Your diplomats await orders.");
}

static unsigned char get_row_y(unsigned char nation_index) {
    return FIRST_ROW_Y + (nation_index * ROW_HEIGHT);
}

static void render_trade_column(unsigned char x, unsigned char y, const unsigned int* resources) {
    unsigned char i;

    for (i = 0; i < FOREIGN_TRADE_ENTRY_COUNT; ++i) {
        print(x, y + i - 1, get_resource_name(resources[i]));
    }
}

static void render_nation_row(GameState* s, unsigned char nation_index) {
    unsigned char y;

    y = get_row_y(nation_index);

    box(BOX_X1, y - 1, BOX_X2, y + 2);
    print_int(NATION_X, y, nation_index + 1);
    print(NATION_X + 1, y, ")");
    print(NATION_X + 2, y, state.foreign_nations[nation_index].name);
    print(RELATION_X, y, get_relation_name(state.foreign_nations[nation_index].relations));
    render_trade_column(EXPORT_X, y, state.foreign_nations[nation_index].exports);
    render_trade_column(IMPORT_X, y, state.foreign_nations[nation_index].imports);
}
