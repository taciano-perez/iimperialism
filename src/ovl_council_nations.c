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
#define FVSTR_COUNCIL_TITLE 31
#define FVSTR_COUNCIL_NATION 32
#define FVSTR_COUNCIL_PROVINCES 33
#define FVSTR_COUNCIL_VOTED_FOR 34
#define FVSTR_COUNCIL_ABSTAINED 35
#define FVSTR_COUNCIL_VOTES_PREFIX 36
#define FVSTR_COUNCIL_COLON 37
#define FVSTR_COUNCIL_VICTORY_CONDITION 38
#define FVSTR_COUNCIL_VICTORY_TARGET 39
#define FVSTR_COUNCIL_WIN 40
#define FVSTR_COUNCIL_ADVICE 41
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

    return get_final_victory_string(FVSTR_COUNCIL_ABSTAINED);
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
    unsigned int firepower;
    unsigned int merchant_capacity;
    char score_line[20];
    const char* text;

    clear_screen();

    text = get_final_victory_string(FRSTR_TITLE);
    print((unsigned char)((40U - strlen(text)) / 2U), 0, text);

    print(0, 2, get_final_victory_string(FRSTR_TREASURY_LABEL));
    print_int_right_aligned_currency(24, 2, state.money);

    firepower = state.frigates * state.guns_per_frigate;
    print(0, 3, get_final_victory_string(FRSTR_SEA_POWER_LABEL));
    print_int(19, 3, firepower);
    print(22, 3, get_final_victory_string(FRSTR_FIREPOWER_SUFFIX));

    merchant_capacity = state.traders * state.capacity_per_trader;
    print(0, 4, get_final_victory_string(FRSTR_MERCHANT_LABEL));
    print_int(19, 4, state.traders);
    print(22, 4, get_final_victory_string(FRSTR_SHIPS_SUFFIX));
    print_int(29, 4, merchant_capacity);
    print(32, 4, get_final_victory_string(FRSTR_CAPACITY_SUFFIX));

    print(0, 5, get_final_victory_string(FRSTR_FOREIGN_LABEL));
    print_int(19, 5, player_nation_votes);
    print(22, 5, get_final_victory_string(FRSTR_FRIENDLY_SUFFIX));

    print(0, 7, get_final_victory_string(FRSTR_TRIUMPHS_PREFIX));
    print_int(19, 7, state.turn_number);
    print(22, 7, get_final_victory_string(FRSTR_TRIUMPHS_SUFFIX));

    build_final_score_line(score_line);
    print_inverted((unsigned char)((40U - strlen(score_line)) / 2U), 9, score_line);

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
            print((unsigned char)((40U - strlen(text)) / 2U), (unsigned char)(21U + row), text);
        }
    }

    play_sound_alert();
    cgetc();
    start_new_game();
}

void render_council_nations_screen(void) {
    unsigned char nation_index;
    unsigned char player_nation_votes;
    unsigned char victory_achieved;
    unsigned char votes_for_name_x;

    clear_screen();
    print(11, 0, get_final_victory_string(FVSTR_COUNCIL_TITLE));

    box(0, 2, 39, 10);
    print(2, 2, get_final_victory_string(FVSTR_COUNCIL_NATION));
    print(12, 2, get_final_victory_string(FVSTR_COUNCIL_PROVINCES));
    print(25, 2, get_final_victory_string(FVSTR_COUNCIL_VOTED_FOR));

    for (nation_index = 0U; nation_index < COUNCIL_NATION_COUNT; ++nation_index) {
        unsigned char row_y;

        row_y = (unsigned char)(4U + nation_index * 1U);

        print(2, row_y, get_council_nation_name(nation_index));
        print_int_right_aligned(20, row_y, get_council_provinces(nation_index));
        print(26, row_y, get_council_vote_target(nation_index));
    }

    player_nation_votes = get_player_nation_council_votes();
    victory_achieved = council_victory_achieved(player_nation_votes);

    print(5, 12, get_final_victory_string(FVSTR_COUNCIL_VOTES_PREFIX));
    votes_for_name_x = (unsigned char)(5U + strlen(get_final_victory_string(FVSTR_COUNCIL_VOTES_PREFIX)));
    print(votes_for_name_x, 12, state.nation_name);
    print((unsigned char)(votes_for_name_x + strlen(state.nation_name)), 12, get_final_victory_string(FVSTR_COUNCIL_COLON));
    print_int_right_aligned(25, 12, player_nation_votes);
    print(5, 13, get_final_victory_string(FVSTR_COUNCIL_VICTORY_CONDITION));
    print(24, 13, get_final_victory_string(FVSTR_COUNCIL_VICTORY_TARGET));

    draw_picture_at(WISEMAN_PORTRAIT, 0, 20);
    play_sound_alert();
    if (victory_achieved) {
        print(5, 20, get_final_victory_string(FVSTR_COUNCIL_WIN));
        wait_three_seconds_or_keypress();
        render_final_report(player_nation_votes);
    } else {
        print(5, 20, get_final_victory_string(FVSTR_COUNCIL_ADVICE));
        state.current_screen = SCREEN_MAIN;
        wait_three_seconds_or_keypress();
    }
}
