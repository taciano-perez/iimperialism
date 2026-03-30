#include "game.h"
#include "strings.h"
#include <conio.h>
#include <string.h>

#define BOX_X1 0
#define BOX_Y1 2
#define BOX_X2 39
#define BOX_Y2 19

#define NATION_X 0
#define RELATION_X 10
#define EXPORT_X 20
#define IMPORT_X 30
#define FIRST_ROW_Y 4
#define ROW_HEIGHT 3

static unsigned char get_row_y(unsigned char nation_index);
static void render_trade_column(unsigned char x, unsigned char y, const unsigned char* resources);
static void render_nation_row(unsigned char nation_index);
static void diplomatic_proposal(unsigned char is_alliance);
static void trade_expedition(void);

void render_diplomacy_screen(void) {
    unsigned char i;
    unsigned char key;

    while (1) {
        clear_screen();
        print(0, 0, "Foreign Office");
        render_turn_funds_header();

        print_inverted(NATION_X, 2, "Nation");
        print_inverted(RELATION_X, 2, "Relations");
        print_inverted(EXPORT_X, 2, "Exports");
        print_inverted(IMPORT_X, 2, "Imports");

        for (i = 0; i < FOREIGN_NATION_COUNT; ++i) {
            render_nation_row(i);
        }

        draw_picture_at(WISEMAN_PORTRAIT, 0, 20);
        print(5, 20, "Launch Trade expedition, offer");
        print(5, 21, "Colony status, Alliance or Quit?");
        key = cgetc_at(37, 21);
        switch (key) {
            case 't':
            case 'T':
                if (state.frigates == 0) {
                    print(5, 22, "We must build warships first!");
                    play_sound_alert();
                    wait_three_seconds_or_keypress();
                    return;
                }
                trade_expedition();
                return;
            case 'a':
            case 'A':
                diplomatic_proposal(RELTYPE_ALLIANCE);
                break;
            case 'c':
            case 'C':
                diplomatic_proposal(RELTYPE_COLONY);
                break;
            case 'q':
            case 'Q':
                state.current_screen = SCREEN_INDUSTRY;
                return;
        }
    }
}

static unsigned char get_row_y(unsigned char nation_index) {
    return FIRST_ROW_Y + (nation_index * ROW_HEIGHT);
}

static void render_trade_column(unsigned char x, unsigned char y, const unsigned char* resources) {
    unsigned char i;

    for (i = 0; i < FOREIGN_TRADE_ENTRY_COUNT; ++i) {
        print(x, y + i - 1, get_resource_name(resources[i]));
    }
}

static void render_nation_row(unsigned char nation_index) {
    unsigned char y;

    y = get_row_y(nation_index);

    box(BOX_X1, y - 1, BOX_X2, y + 2);
    print_int(NATION_X, y, nation_index + 1);
    print(NATION_X + 1, y, ")");
    print(NATION_X + 2, y, state.foreign_nations[nation_index].name);
    print(RELATION_X, y, get_relation_name(state.foreign_nations[nation_index].relations, nation_index));
    render_trade_column(EXPORT_X, y, state.foreign_nations[nation_index].exports);
    render_trade_column(IMPORT_X, y, state.foreign_nations[nation_index].imports);
}

static void diplomatic_proposal(unsigned char is_alliance) {
    unsigned char nation_index;
    unsigned char start_index;
    unsigned char end_index;
    unsigned char x;
    unsigned char printed_any;

    clear_input_area();

    if (is_alliance) {
        start_index = 0U;
        end_index = 1U;
    } else {
        start_index = 2U;
        end_index = 4U;
    }

    clear_area(5, 21, 34, 1);

    x = 5U;
    printed_any = FALSE;
    for (nation_index = start_index; nation_index <= end_index; ++nation_index) {
        if (state.foreign_nations[nation_index].relations < RELATION_EXCELLENT || state.foreign_nations[nation_index].relations == RELATION_ALLY_COLONY) {
            continue;
        }

        if (printed_any) {
            print(x, 21, ", ");
            x = (unsigned char)(x + 2U);
        }

        print_int(x, 21, nation_index + 1U);
        print(x+1, 21, ")");
        x = (unsigned char)(x + 2U);
        print(x, 21, state.foreign_nations[nation_index].name);
        x = (unsigned char)(x + strlen(state.foreign_nations[nation_index].name));
        printed_any = TRUE;
    }

    if (printed_any) {
        print(5, 20, "Nation to offer");
        if (is_alliance) {
            print(21, 20, "alliance?");
        } else {
            print(21, 20, "colony state?");
        }
        while (1) {
            nation_index = scan_uint(5, 22, 1)-1;
            if (nation_index < FOREIGN_NATION_COUNT && state.foreign_nations[nation_index].relations >= RELATION_EXCELLENT) {
                // make acceptance random and print out message
                state.foreign_nations[nation_index].relations = RELATION_ALLY_COLONY;
                break;
            }
        }
    } else {
        // FIXME: we may land here also if all nations are already allies/colonies.
        print(5, 20, "You need excellent relations with");
        if (is_alliance) {
            print(5, 21, "a great power to offer alliance.");
        } else {
            print(5, 21, "a minor nation to offer colony status.");
        }
        play_sound_alert();
        wait_three_seconds_or_keypress();
    }
}

static void trade_expedition(void) {
    unsigned int selection;
    unsigned char nation_index;

    while (1) {
        clear_input_area();
        print(5, 20, "Which nation to trade (1-5)?");

        selection = scan_uint(36, 20, 1);
        if (selection < 1 || selection > FOREIGN_NATION_COUNT) {
            continue;
        }
        nation_index = (unsigned char)(selection - 1);

        print(5, 22, "Sailing to the Sea of");
        print(27, 22, state.foreign_nations[nation_index].name);
        print(33, 22, "...");
        wait_three_seconds_or_keypress();
        set_selected_trade_nation(nation_index);
        if (rand_range(1U, 100U) <= TRADE_EXPEDITION_BATTLE_CHANCE_PERCENT) {
            state.current_screen = SCREEN_BATTLE;
        } else {
            state.current_screen = SCREEN_TRADE_EXPEDITION;
        }
        return;
    }
}
