#include "game.h"
#include "strings.h"
#include <string.h>

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
#define FIRST_COL_X 5U

#define DSTR_TITLE 0
#define DSTR_NATION 1
#define DSTR_STATUS 2
#define DSTR_EXPORTS 3
#define DSTR_IMPORTS 4
#define DSTR_PROMPT_1 5
#define DSTR_PROMPT_2 6
#define DSTR_NEED_WARSHIPS 7
#define DSTR_DIPLOMATIC_COST 8
#define DSTR_OFFER_NATION 9
#define DSTR_ALLIANCE 10
#define DSTR_COLONY_STATE 11
#define DSTR_ACCEPTED 12
#define DSTR_REJECTED 13
#define DSTR_NEED_GREAT 14
#define DSTR_NEED_MONEY 15
#define DSTR_NEED_ALLIANCE 16
#define DSTR_NEED_COLONY 17
#define DSTR_TRADE_WHICH 18
#define DSTR_SAILING 19
#define DSTR_ELLIPSIS 20

#define DSTR(id) get_diplomacy_string(id)

static void diplomatic_proposal(unsigned char is_alliance);
static void trade_expedition(void);

void render_diplomacy_screen(void) {
    unsigned char i;
    unsigned char j;
    unsigned char key;
    unsigned char y;
    ForeignNation* nation;

    while (1) {
        clear_screen();
        print(0, 0, DSTR(DSTR_TITLE));
        render_turn_funds_header();

        print_inverted(NATION_X, 2, DSTR(DSTR_NATION));
        print_inverted(RELATION_X, 2, DSTR(DSTR_STATUS));
        print_inverted(EXPORT_X, 2, DSTR(DSTR_EXPORTS));
        print_inverted(IMPORT_X, 2, DSTR(DSTR_IMPORTS));

        y = FIRST_ROW_Y;
        for (i = 0; i < FOREIGN_NATION_COUNT; ++i) {
            nation = &state.foreign_nations[i];
            box(BOX_X1, y - 1, BOX_X2, y + 2);
            print_int(NATION_X, y, i + 1);
            print(NATION_X + 1, y, ")");
            print(NATION_X + 2, y, nation->name);
            print(RELATION_X, y, get_relation_name(nation->relations, i));
            if (nation->relations != RELATION_ALLY_COLONY) {
                print_bold(RELATION_X+6, y, (nation->relations_previous_turn > nation->relations) ? "~" : (nation->relations_previous_turn < nation->relations) ? "^" : " ");
            }
            for (j = 0; j < FOREIGN_TRADE_ENTRY_COUNT; ++j) {
                print(EXPORT_X, y + j - 1, get_resource_name(nation->exports[j]));
                print(IMPORT_X, y + j - 1, get_resource_name(nation->imports[j]));
            }
            y = (unsigned char)(y + ROW_HEIGHT);
        }

        draw_picture_at(WISEMAN_PORTRAIT, 0, 20);
        print(5, 20, DSTR(DSTR_PROMPT_1));
        print(5, 21, DSTR(DSTR_PROMPT_2));
        key = cgetc_at(37, 21);
        switch (key) {
            case 't':
            case 'T':
                if (state.frigates == 0) {
                    print(5, 22, DSTR(DSTR_NEED_WARSHIPS));
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
                state.current_screen = SCREEN_MAIN;
                return;
        }
    }
}

static void diplomatic_proposal(unsigned char is_alliance) {
    unsigned char nation_index = (unsigned char)(is_alliance ? 0U : 2U);
    unsigned char x;
    unsigned char end_index = (unsigned char)(is_alliance ? 2U : FOREIGN_NATION_COUNT);
    ForeignNation* nation;

    clear_input_area();
    clear_area(5, 21, 34, 1);

    x = FIRST_COL_X;
    for (; nation_index < end_index; ++nation_index) {
        nation = &state.foreign_nations[nation_index];
        if (nation->relations < RELATION_EXCELLENT || nation->relations == RELATION_ALLY_COLONY) {
            continue;
        }

        if (x != FIRST_COL_X) {
            print(x, 22, ",");
            x = (unsigned char)(x + 1U);
        }

        print_int(x, 22, nation_index + 1U);
        print(x + 1, 22, ")");
        x = (unsigned char)(x + 2U);
        print(x, 22, nation->name);
        x = (unsigned char)(x + strlen(nation->name));
        print(x, 22, ",");
    }
    print(x+1, 22, "0)Quit?");

    if (x != FIRST_COL_X || state.money >= DIPLOMATIC_OFFER_COST) {
        print(5, 20, DSTR(DSTR_DIPLOMATIC_COST));
        print(5, 21, DSTR(DSTR_OFFER_NATION));
        print(21, 21, DSTR(is_alliance ? DSTR_ALLIANCE : DSTR_COLONY_STATE));
        while (1) {
            nation_index = (unsigned char)(scan_uint(x+8, 22, 1) - 1U);
            if (nation_index == 255U) { // user typed 0 to quit
                break; 
            }
            if (nation_index < FOREIGN_NATION_COUNT && state.foreign_nations[nation_index].relations >= RELATION_EXCELLENT) {
                clear_input_area();
                if (rand_range(1U, 100U) <= DIPLOMATIC_OVERTURE_CHANCE_PERCENT) {
                    state.foreign_nations[nation_index].relations = RELATION_ALLY_COLONY;
                    print(5, 21, DSTR(DSTR_ACCEPTED));
                } else {
                    print(5, 21, DSTR(DSTR_REJECTED));
                }
                state.money = state.money - DIPLOMATIC_OFFER_COST;
                play_sound_alert();
                wait_three_seconds_or_keypress();
                break;
            }
        }
    } else {
        print(5, 20, DSTR(DSTR_NEED_GREAT));
        print(5, 21, DSTR(DSTR_NEED_MONEY));
        print(5, 22, DSTR(is_alliance ? DSTR_NEED_ALLIANCE : DSTR_NEED_COLONY));
        play_sound_alert();
        wait_three_seconds_or_keypress();
    }
}

static void trade_expedition(void) {
    unsigned char nation_index;

    while (1) {
        clear_input_area();
        print(5, 20, DSTR(DSTR_TRADE_WHICH));

        nation_index = (unsigned char)(scan_uint(36, 20, 1) - 1U);
        if (nation_index >= FOREIGN_NATION_COUNT) {
            continue;
        }

        print(5, 22, DSTR(DSTR_SAILING));
        print(27, 22, state.foreign_nations[nation_index].name);
        print(33, 22, DSTR(DSTR_ELLIPSIS));
        wait_three_seconds_or_keypress();
        set_selected_trade_nation(nation_index);
        if (rand_range(1U, 100U) <= TRADE_EXPEDITION_BATTLE_CHANCE_PERCENT) {
            state.attacker_index = INDEX_PIRATES;
            state.current_screen = SCREEN_BATTLE;
        } else {
            unsigned char i;
            for (i = 0; i < FOREIGN_NATION_COUNT; ++i) {
                if (state.foreign_nations[i].relations == RELATION_BAD && rand_range(1U, 100U) <= TRADE_EXPEDITION_ATTACK_FOREIGN_NATION_CHANCE_PERCENT) {
                    state.attacker_index = i;
                    state.current_screen = SCREEN_BATTLE;
                    return;
                }
            }
            state.current_screen = SCREEN_TRADE_EXPEDITION;
        }
        return;
    }
}
