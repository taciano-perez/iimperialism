#include <conio.h>
#include <string.h>
#include "game.h"

static const unsigned char QUOTE_BASE[SCORE_RANK_COUNT] = { 21U, 23U, 25U, 27U, 29U };
static const unsigned char QUOTE_COUNT[SCORE_RANK_COUNT] = { 2U, 2U, 2U, 2U, 2U };

#define FRSTR_TITLE 0
#define FRSTR_TREASURY_LABEL 1
#define FRSTR_SEA_POWER_LABEL 2
#define FRSTR_MERCHANT_LABEL 3
#define FRSTR_FOREIGN_LABEL 4
#define FRSTR_TRIUMPHS_PREFIX 5
#define FRSTR_TRIUMPHS_SUFFIX 6
#define FRSTR_FRIENDLY_SUFFIX 7
#define FRSTR_FIREPOWER_SUFFIX 8
#define FRSTR_SHIPS_SUFFIX 9
#define FRSTR_CAPACITY_SUFFIX 10

#define FRSTR_RANK_NAME_BASE 11
#define FRSTR_RANK_RANGE_BASE 16

#define STR_COUNCIL_VOTES_PREFIX "Votes for "

static const char* get_council_nation_name(unsigned char nation_index);
static unsigned char get_council_provinces(unsigned char nation_index);
static const char* get_council_vote_target(unsigned char nation_index);
static unsigned char get_player_nation_council_votes(void);
static unsigned char council_victory_achieved(unsigned char player_nation_votes);
static const char* get_rank_quote_line(unsigned char rank_index, unsigned char line_index);
static void render_final_report(unsigned char player_nation_votes);

static const char* get_council_nation_name(unsigned char nation_index) {
    if (nation_index == 0U) {
        return state.nation_name;
    }

    return state.foreign_nations[nation_index - 1U].name;
}

static unsigned char get_council_provinces(unsigned char nation_index) {
    if (nation_index < 3U) {
        return 8U;
    }

    return 4U;
}

static const char* get_council_vote_target(unsigned char nation_index) {
    if (nation_index == 0U) {
        return state.nation_name;
    }

    if (state.foreign_nations[nation_index - 1U].relations == RELATION_ALLY_COLONY) {
        return state.nation_name;
    }

    if (nation_index < 3U) {
        return get_council_nation_name(nation_index);
    }

    return "Abstained";
}

static unsigned char get_player_nation_council_votes(void) {
    unsigned char nation_index;
    unsigned char player_nation_votes;

    player_nation_votes = 0U;
    for (nation_index = 0U; nation_index < COUNCIL_NATION_COUNT; ++nation_index) {
        if (get_council_vote_target(nation_index) == state.nation_name) {
            player_nation_votes = (unsigned char)(player_nation_votes + get_council_provinces(nation_index));
        }
    }

    return player_nation_votes;
}

static unsigned char council_victory_achieved(unsigned char player_nation_votes) {
    return player_nation_votes >= COUNCIL_VICTORY_VOTES;
}

static const char* get_rank_quote_line(unsigned char rank_index, unsigned char line_index) {
    if (line_index >= QUOTE_COUNT[rank_index]) {
        return "";
    }

    return get_final_victory_string((unsigned char)(QUOTE_BASE[rank_index] + line_index));
}

static void render_final_report(unsigned char player_nation_votes) {
    unsigned char rank_index;
    unsigned char row;
    unsigned char key;
    unsigned int firepower;
    unsigned int merchant_capacity;
    char score_line[20];
    const char* text;

    clear_screen();

    print(7, 0, get_final_victory_string(FRSTR_TITLE));
    print_bold((unsigned char)((40U - strlen(state.nation_name)) / 2U), 1, state.nation_name);

    print(0, 3, get_final_victory_string(FRSTR_TREASURY_LABEL));
    print_int_right_aligned_currency(24, 3, state.money);

    firepower = state.frigates * state.guns_per_frigate;
    print(0, 4, get_final_victory_string(FRSTR_SEA_POWER_LABEL));
    print_int(19, 4, firepower);
    print(22, 4, get_final_victory_string(FRSTR_FIREPOWER_SUFFIX));

    merchant_capacity = state.traders * state.capacity_per_trader;
    print(0, 5, get_final_victory_string(FRSTR_MERCHANT_LABEL));
    print_int(19, 5, state.traders);
    print(22, 5, get_final_victory_string(FRSTR_SHIPS_SUFFIX));
    print_int(29, 5, merchant_capacity);
    print(32, 5, get_final_victory_string(FRSTR_CAPACITY_SUFFIX));

    print(0, 6, get_final_victory_string(FRSTR_FOREIGN_LABEL));
    print_int(19, 6, player_nation_votes);
    print(22, 6, get_final_victory_string(FRSTR_FRIENDLY_SUFFIX));

    print(0, 8, get_final_victory_string(FRSTR_TRIUMPHS_PREFIX));
    print_int(19, 8, state.turn_number);
    print(22, 8, get_final_victory_string(FRSTR_TRIUMPHS_SUFFIX));

    build_final_score_line(score_line);
    print_inverted((unsigned char)((40U - strlen(score_line)) / 2U), 10, score_line);

    box(0, 12, 39, 19);

    rank_index = get_final_rank_index();
    for (row = 0U; row < SCORE_RANK_COUNT; ++row) {
        unsigned char y;

        y = (unsigned char)(13U + row);
        if (row == rank_index) {
            print_inverted(2, y, get_final_victory_string((unsigned char)(FRSTR_RANK_NAME_BASE + row)));
        } else {
            print(2, y, get_final_victory_string((unsigned char)(FRSTR_RANK_NAME_BASE + row)));
        }
        print(23, y, get_final_victory_string((unsigned char)(FRSTR_RANK_RANGE_BASE + row)));
    }

    for (row = 0U; row < 3U; ++row) {
        text = get_rank_quote_line(rank_index, row);
        if (text[0] != '\0') {
            print((unsigned char)((40U - strlen(text)) / 2U), (unsigned char)(20U + row), text);
        }
    }

    print(0, 23, "Play again?");
    while (1) {
        key = cgetc_at(11, 23);
        if (key == 'Y' || key == 'y') {
            start_new_game();
            return;
        }
    }
    
}

void ask_continue_question() {
    unsigned char key;

    print(5, 21, "Proceed?");
    while (1) {
        key = cgetc_at(13, 21);
        if (key == 'P' || key == 'p' || key == 'Y' || key == 'y') {
            return;
        }
    }
}

void render_council_nations_screen(void) {
    unsigned char nation_index;
    unsigned char player_nation_votes;
    unsigned char victory_achieved;

    clear_screen();
    print(11, 0, "Council of Nations");

    box(0, 4, 39, 10);
    print_inverted(2, 3, "Nation");
    print_inverted(12, 3, "Provinces");
    print_inverted(26, 3, "Voted for");

    for (nation_index = 0U; nation_index < COUNCIL_NATION_COUNT; ++nation_index) {
        unsigned char row_y;

        row_y = (unsigned char)(4U + nation_index * 1U);

        print(2, row_y, get_council_nation_name(nation_index));
        print_int_right_aligned(20, row_y, get_council_provinces(nation_index));
        print(26, row_y, get_council_vote_target(nation_index));
    }

    player_nation_votes = get_player_nation_council_votes();
    victory_achieved = council_victory_achieved(player_nation_votes);

    print(5, 13, "Votes for ");
    print(15, 13, state.nation_name);
    print((unsigned char)(15 + strlen(state.nation_name)), 13, ":");
    print_int_right_aligned(27, 13, player_nation_votes);
    print(5, 15, "Victory Condition:");
    print(26, 15, "24 votes");

    draw_picture_at(CHANCELLOR_PORTRAIT, 0, 20);
    play_sound_alert();
    state.current_screen = SCREEN_MAIN;
    if (victory_achieved) {
        print(5, 20, "You are victorious!");
        ask_continue_question();
        render_final_report(player_nation_votes);
    } else {
        print(5, 20, "Keep trading to improve relations.");
        ask_continue_question();
    }
}
