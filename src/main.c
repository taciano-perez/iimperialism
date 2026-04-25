#include <conio.h>
#include "sound.h"
#include "overlay.h"
#include "game.h"
#include "strings.h"

#define BOX1_X1 0
#define BOX1_Y1 2
#define BOX1_X2 39
#define BOX1_Y2 7
#define CHAR_HEIGHT 8

// Global state instance
GameState state;
static unsigned char selected_trade_nation;

static unsigned int wait_for_splash_escape(void);
static void prompt_for_nation_name(char* nation_name, unsigned char max_length);
static unsigned int get_assigned_workers(void);
static unsigned char get_rating_tier(unsigned int value, unsigned int maximum);
static unsigned char get_industry_rating_tier(void);
static unsigned char get_science_rating_tier(void);
static unsigned char get_merchant_marine_rating_tier(void);
static unsigned char get_navy_rating_tier(void);
static unsigned char get_diplomacy_rating_tier(void);
static const char* get_rating_name(unsigned char tier);
static void print_rating_row(unsigned char x, unsigned char y, unsigned char right_x, const char* label, unsigned char tier);

void render_warehouse_box() {
    box(BOX1_X1, BOX1_Y1+1, BOX1_X2, BOX1_Y2);
    print_inverted((BOX1_X1), BOX1_Y1, "Warehouse");

    print((BOX1_X1+1), (BOX1_Y1+1), "Timber:");
    print_int_right_aligned((BOX1_X1+11), (BOX1_Y1+1), state.resources[RESOURCE_TIMBER]);
    print((BOX1_X1+1), (BOX1_Y1+2), "Wool:");
    print_int_right_aligned((BOX1_X1+11), (BOX1_Y1+2), state.resources[RESOURCE_WOOL]);
    print((BOX1_X1+1), (BOX1_Y1+3), "Iron:");
    print_int_right_aligned((BOX1_X1+11), (BOX1_Y1+3), state.resources[RESOURCE_IRON]);
    print((BOX1_X1+1), (BOX1_Y1+4), "Coal:");
    print_int_right_aligned((BOX1_X1+11), (BOX1_Y1+4), state.resources[RESOURCE_COAL]);

    print((BOX1_X1+13), (BOX1_Y1+1), "Lumber:");
    print_int_right_aligned((BOX1_X1+23), (BOX1_Y1+1), state.resources[RESOURCE_LUMBER]);
    print((BOX1_X1+13), (BOX1_Y1+2), "Fabric:");
    print_int_right_aligned((BOX1_X1+23), (BOX1_Y1+2), state.resources[RESOURCE_FABRIC]);
    print((BOX1_X1+13), (BOX1_Y1+3), "Steel:");
    print_int_right_aligned((BOX1_X1+23), (BOX1_Y1+3), state.resources[RESOURCE_STEEL]);

    print((BOX1_X1+25), (BOX1_Y1+1), "Furniture:");
    print_int_right_aligned((BOX1_X1+38), (BOX1_Y1+1), state.resources[RESOURCE_FURNITURE]);
    print((BOX1_X1+25), (BOX1_Y1+2), "Clothes:");
    print_int_right_aligned((BOX1_X1+38), (BOX1_Y1+2), state.resources[RESOURCE_CLOTHES]);
    print((BOX1_X1+25), (BOX1_Y1+3), "Tools:");
    print_int_right_aligned((BOX1_X1+38), (BOX1_Y1+3), state.resources[RESOURCE_TOOLS]);
    print((BOX1_X1+25), (BOX1_Y1+4), "Guns:");
    print_int_right_aligned((BOX1_X1+38), (BOX1_Y1+4), state.resources[RESOURCE_GUNS]);
}

void render_turn_funds_header(void) {
    print(28, 0, "Turn:");
    print_int_right_aligned(39, 0, state.turn_number);
    print(27, 1, "Funds:");
    print_int_right_aligned_currency(39, 1, state.money);
}

void set_selected_trade_nation(unsigned char nation_index) {
    selected_trade_nation = nation_index;
}

unsigned char get_selected_trade_nation(void) {
    return selected_trade_nation;
}

static unsigned int wait_for_splash_escape(void) {
    char key;
    unsigned int entropy = 0xA55AU;

    while (1) {
        entropy = (unsigned int)((entropy << 1) ^ (entropy >> 1) ^ 0xB400U ^ 0x003DU);

        if (kbhit()) {
            key = cgetc();
            if (key == 27) {
                break;
            }
            entropy ^= (unsigned char)key;
        }
    }

    return entropy;
}

static void prompt_for_nation_name(char* nation_name, unsigned char max_length) {
    clear_screen();
    box(0, 8, 39, 16);
    // print(5, 9, "Your Excellency,");
    print(4, 11, "What is the name of your");
    print(5, 13, "nation?");

    print(13, 14, "----------");
    scan_text(13, 13, nation_name, max_length);
}

static unsigned int get_assigned_workers(void) {
    return state.production_lumber + state.production_fabric + state.production_steel
         + state.production_furniture + state.production_clothes
         + state.production_tools + state.production_guns;
}

static unsigned char get_rating_tier(unsigned int value, unsigned int maximum) {
    if (value == 0U || maximum == 0U) {
        return 0U;
    }

    if (value >= maximum) {
        return 4U;
    }

    return (unsigned char)((value * 4U + (maximum / 2U)) / maximum);
}

static unsigned char get_industry_rating_tier(void) {
    unsigned int used_workers;

    used_workers = get_assigned_workers();
    return get_rating_tier(used_workers, used_workers + state.available_workers);
}

static unsigned char get_science_rating_tier(void) {
    return get_rating_tier(state.science_level, SCIENCE_LEVEL_COUNT - 1U);
}

static unsigned char get_merchant_marine_rating_tier(void) {
    unsigned int total_workers;

    total_workers = state.available_workers + get_assigned_workers();
    return get_rating_tier(state.traders * state.capacity_per_trader, total_workers);
}

static unsigned char get_navy_rating_tier(void) {
    return get_rating_tier(state.warships, 12U);
}

static unsigned char get_diplomacy_rating_tier(void) {
    unsigned char i;
    unsigned char allied_votes;
    unsigned char relation_score;

    allied_votes = 0U;
    relation_score = 0U;

    for (i = 0U; i < FOREIGN_NATION_COUNT; ++i) {
        if (state.foreign_nations[i].relations == RELATION_ALLY_COLONY) {
            allied_votes = (unsigned char)(allied_votes + ((i < 2U) ? 8U : 4U));
        }
        relation_score = (unsigned char)(relation_score + get_relation_tier(state.foreign_nations[i].relations));
    }

    if (allied_votes >= 16U) {
        return 4U;
    }

    return MIN(3U, get_rating_tier(relation_score, FOREIGN_NATION_COUNT * 4U));
}

static const char* get_rating_name(unsigned char tier) {
    switch (tier) {
        case 0U: return STR_RELATION_BAD;
        case 1U: return STR_RELATION_POOR;
        case 2U: return STR_RELATION_FAIR;
        case 3U: return STR_RELATION_GOOD;
        default: return STR_RELATION_GREAT;
    }
}

static void print_rating_row(unsigned char x, unsigned char y, unsigned char right_x, const char* label, unsigned char tier) {
    print(x, y, label);
    print_right_aligned(right_x, y, get_rating_name(tier));
}

void start_new_game(void) {
    char nation_name[11];

    prompt_for_nation_name(nation_name, 10U);
    copy_text_limited(state.nation_name, nation_name, sizeof(state.nation_name));
    init_game();
}

void render_main_screen(void) {
    char key;

    clear_screen();
    print(0, 0, "Nation of");
    print_bold(10,  0, state.nation_name);
    render_turn_funds_header();

    box(0, 3, 39, 9);
    print_inverted(1, 3, "Industry Minister");
    draw_picture_offset_at(INDUSTRY_PORTRAIT, 0, 4U, 5);
    print_rating_row(5, 6, 17, "Economy:", get_industry_rating_tier());

    box(19, 3, 39, 9);
    print_inverted(20, 3, "Science Minister");
    draw_picture_at(SCIENCE_PORTRAIT, 20, 5);
    print_rating_row(24, 6, 38, "Research:", get_science_rating_tier());

    box(0, 11, 39, 17);
    print_inverted(1, 11, "Admiral");
    draw_picture_offset_at(ADMIRAL_PORTRAIT, 0, 4U, 13);
    print(5, 13, "Merchant");
    print_rating_row(5, 14, 17, "Marine:", get_merchant_marine_rating_tier());
    print_rating_row(5, 16, 17, "Navy:", get_navy_rating_tier());

    box(19, 11, 39, 17);
    print_inverted(20, 11, "Chancellor");
    draw_picture_at(CHANCELLOR_PORTRAIT, 20, 13);
    print_rating_row(24, 13, 38, "Diplomacy:", get_diplomacy_rating_tier());

    print(0, 20, "Visit Ministry of Industry, Foreign");
    print(0, 21, "Office, Admiralty, Science Academy,");
    print(0, 22, "or End turn?");

    while (1) {
        key = cgetc_at(12, 22);
        switch (key) {
            case 'i':
            case 'I':
                state.current_screen = SCREEN_INDUSTRY;
                return;
            case 's':
            case 'S':
                state.current_screen = SCREEN_SCIENCE;
                return;
            case 'a':
            case 'A':
                state.current_screen = SCREEN_ADMIRALTY;
                return;
            case 'f':
            case 'F':
                state.current_screen = SCREEN_DIPLOMACY;
                return;
            case 'e':
            case 'E':
                next_turn();
                return;
            case 27: // ESC key
                run_overlay(OVL_GAME_MENU);
                return;
        }
    }
}

int main(void) {
    clrscr();
    cputsxy(16, 11, "LOADING");
    init_overlays();
    ui_init();

    // splash screen
    clear_screen();
    print_bold(1, 1, "I I M P E R I A L I S M !");
    print(1, 2, "-------------------------");
    print(0, 3, "A game based on the European");
    print(2, 4, "expansion of the 1800's");
    draw_picture_at(SHIP_SPLASH, 1, 10);
    print(28, 6, "Created By:");
    print(29, 8, "Taciano D.");
    print(30, 9, "Perez");
    print_bold(28, 11, "===========");
    print(29, 13, "Copyright");
    print(29, 14, "(c) 2026");
    print_bold(28, 16, "===========");
    print(29, 18, "Press the");
    print_bold(29, 19, "`ESC`");
    print(34, 19, "key");
    print(29, 20, "to start.");
    seed_random(wait_for_splash_escape());
    start_new_game();

    while (1) { // main game loop
        switch (state.current_screen) {
            case SCREEN_MAIN:
                render_main_screen();
                continue;

            case SCREEN_INDUSTRY:
                run_overlay(OVL_INDUSTRY);
                continue;

            case SCREEN_TRANSPORT:
                run_overlay(OVL_TRANSPORT);
                continue;

            case SCREEN_PRODUCTION:
                run_overlay(OVL_PRODUCTION);
                continue;

            case SCREEN_ADMIRALTY:
                run_overlay(OVL_ADMIRALTY);
                continue;

            case SCREEN_DIPLOMACY:
                run_overlay(OVL_DIPLOMACY);
                continue; // skip input handling and go directly to next screen

            case SCREEN_TRADE_EXPEDITION:
                render_trade_market();
                run_overlay(OVL_TRADE_EXPEDITION_ACTION);
                continue;

            case SCREEN_BATTLE:
                clear_screen();
                draw_picture_at(ADMIRAL_PORTRAIT, 0, 20);
                print(5, 20, "Battle at sea!");
                if (state.attacker_index == INDEX_PIRATES) {
                    print(20, 20, "Pirate attack!");
                } else {
                    print(20, 20, "Warships from");
                    print(34, 20, state.foreign_nations[state.attacker_index].name);
                }
                print(0, 0, "Our Navy");
                print(23, 0, "Enemy Fleet");
                play_sound_alert();
                run_overlay(OVL_BATTLE);
                continue;

            case SCREEN_SCIENCE:
                run_overlay(OVL_SCIENCE);
                continue;

            case SCREEN_COUNCIL_NATIONS:
                run_overlay(OVL_COUNCIL_NATIONS);
                continue;

            case SCREEN_LEDGER:
                run_overlay(OVL_INDUSTRY);
                continue;
        }
    }

}
