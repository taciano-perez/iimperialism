#include <conio.h>
#include <string.h>
#include "game.h"

static const char STR_TITLE[] = "Council of Nations";
static const char STR_NATION[] = "Nation";
static const char STR_PROVINCES[] = "Provinces";
static const char STR_VOTED_FOR[] = "Voted for";
static const char STR_ABSTAINED[] = "Abstained";
static const char STR_VOTES_FOR_PREFIX[] = "Votes for ";
static const char STR_COLON[] = ":";
static const char STR_VICTORY_CONDITION[] = "Victory Condition:";
static const char STR_VICTORY_TARGET[] = "24 votes";
static const char STR_WIN[] = "You have won the game!";
static const char STR_ADVICE_1[] = "Keep trading to improve relations";
static const char STR_ADVICE_2[] = "and win the council's favor.";
static const unsigned char COUNCIL_NATION_COUNT = 6U;
static const unsigned char COUNCIL_VICTORY_VOTES = 24U;

static const char* get_council_nation_name(unsigned char nation_index);
static unsigned char get_council_provinces(unsigned char nation_index);
static const char* get_council_vote_target(unsigned char nation_index);
static unsigned char get_player_nation_council_votes(void);
static unsigned char council_victory_achieved(unsigned char player_nation_votes);

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

    return STR_ABSTAINED;
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

void render_council_nations_screen(void) {
    unsigned char nation_index;
    unsigned char player_nation_votes;
    unsigned char victory_achieved;
    unsigned char votes_for_name_x;

    clear_screen();
    print(11, 0, STR_TITLE);

    box(0, 2, 39, 10);
    print(2, 2, STR_NATION);
    print(12, 2, STR_PROVINCES);
    print(25, 2, STR_VOTED_FOR);

    for (nation_index = 0U; nation_index < COUNCIL_NATION_COUNT; ++nation_index) {
        unsigned char row_y;

        row_y = (unsigned char)(4U + nation_index * 1U);

        print(2, row_y, get_council_nation_name(nation_index));
        print_int_right_aligned(20, row_y, get_council_provinces(nation_index));
        print(26, row_y, get_council_vote_target(nation_index));
    }

    player_nation_votes = get_player_nation_council_votes();
    victory_achieved = council_victory_achieved(player_nation_votes);

    print(5, 12, STR_VOTES_FOR_PREFIX);
    votes_for_name_x = (unsigned char)(5U + strlen(STR_VOTES_FOR_PREFIX));
    print(votes_for_name_x, 12, state.nation_name);
    print((unsigned char)(votes_for_name_x + strlen(state.nation_name)), 12, STR_COLON);
    print_int_right_aligned(25, 12, player_nation_votes);
    print(5, 13, STR_VICTORY_CONDITION);
    print(24, 13, STR_VICTORY_TARGET);

    draw_picture_at(WISEMAN_PORTRAIT, 0, 20);
    if (victory_achieved) {
        print(5, 20, STR_WIN);
        cgetc();
    } else {
        print(5, 20, STR_ADVICE_1);
        print(5, 21, STR_ADVICE_2);
        play_sound_alert();
        print_bold(7, 23, "Press any key to continue...");
        cgetc();
        state.current_screen = SCREEN_MAIN;
    }
}
