#include "game.h"

static const char TECH_1[] = "1)Railways";
static const char TECH_2[] = "2)Carronade";
static const char TECH_3[] = "3)Clipper ships";
static const char TECH_4[] = "4)Telegraph";
static const char TECH_5[] = "5)Shell guns";
static const char TECH_6[] = "6)Steel hulls";
static const char* const TECH_NAMES[] = {
    TECH_1, TECH_2, TECH_3, TECH_4, TECH_5, TECH_6
};

static const char DESC_1[] = "2x wagon capacity";
static const char DESC_2[] = "2x guns per warship";
static const char DESC_3[] = "2x trader capacity";
static const char DESC_4[] = "4x wagon capacity";
static const char DESC_5[] = "4x guns per warship";
static const char DESC_6[] = "4x trader capacity";
static const char* const TECH_DESCRIPTIONS[] = {
    DESC_1, DESC_2, DESC_3, DESC_4, DESC_5, DESC_6
};

static const char STR_PATENTED[] = "Patented";
static const char STR_PROMPT_RESEARCH[] = "Invest in Research or Quit?";
static const char STR_PROMPT_FUNDS[] = "Insufficient research funds. Quit?";
static const char STR_PROMPT_DONE[] = "All research patented. Quit?";

static void render_technology_row(unsigned char level, unsigned char row, unsigned char patented) {
    print(1, row, TECH_NAMES[level - 1]);
    if (patented) {
        print(30, row, STR_PATENTED);
    } else {
        print_int_right_aligned_currency(38, row, (unsigned int) level * SCIENCE_RESEARCH_COST_MULTIPLIER);
    }
    print(15, row + 1, TECH_DESCRIPTIONS[level - 1]);
}

void render_science_screen(void) {
    while (1) {
        unsigned char next_level;
        unsigned int next_cost;

        clear_screen();
        print(0, 0, "Patent Office");
        render_turn_funds_header();
        
        print(1, 3, "Research");
        print(35, 3, "Cost");
        box(0, 4, 39, 16);

        next_level = state.science_level + 1U;
        {
            unsigned char level;
            unsigned char row;

            row = 4;
            for (level = 1U; level <= state.science_level; ++level) {
                render_technology_row(level, row, 1U);
                row += 2;
            }

            if (next_level < SCIENCE_LEVEL_COUNT) {
                render_technology_row(next_level, row, 0U);
            }
        }

        /* ADVISOR */
        draw_picture_at(SCIENCE_PORTRAIT, 0, 20);
        if (next_level >= SCIENCE_LEVEL_COUNT) {
            print(5, 20, STR_PROMPT_DONE);
            next_cost = 0U;
        } else {
            next_cost = (unsigned int) next_level * SCIENCE_RESEARCH_COST_MULTIPLIER;
            if (state.money >= next_cost) {
                print(5, 20, STR_PROMPT_RESEARCH);
            } else {
                print(5, 20, STR_PROMPT_FUNDS);
            }
        }

        while (1) {
            unsigned char key;
            key = cgetc_at(32, 20);
            switch (key) {
                case 'R':
                case 'r':
                    if (next_level < SCIENCE_LEVEL_COUNT && state.money >= next_cost) {
                        state.money -= next_cost;
                        state.science_level = next_level;
                        if (next_level == 2U) {
                            state.guns_per_frigate = GUNS_PER_FRIGATE_BASE * 2U;
                        } else if (next_level == 3U) {
                            state.capacity_per_trader = CAPACITY_PER_TRADER_BASE * 2U;
                        } else if (next_level == 5U) {
                            state.guns_per_frigate = GUNS_PER_FRIGATE_BASE * 4U;
                        } else if (next_level == 6U) {
                            state.capacity_per_trader = CAPACITY_PER_TRADER_BASE * 4U;
                        }
                        break;
                    }
                    continue;
                case 'Q':
                case 'q':
                    state.current_screen = SCREEN_MAIN;
                    return;
            }
            break;
        }
    }
}
