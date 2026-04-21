#include "game.h"

#pragma code-name (push, "LOWCODE")

static unsigned char get_resource_base_price(unsigned char resource);
static unsigned int apply_percent(unsigned int value, unsigned char percent);
static void add_resource_saturating(unsigned char resource, unsigned char amount);
static void update_foreign_nation_prices(ForeignNation* nation);
static void assign_foreign_nation_names(void);
static unsigned char rand_resource_excluding(unsigned char min, unsigned char max, unsigned char exclude);
static unsigned char strings_equal(const char* left, const char* right);
static char* append_uint_decimal(char* buffer, unsigned int value);
static unsigned int get_final_score(void);
static unsigned char get_final_friendly_provinces(void);
static unsigned char get_final_treasury_score(void);
static unsigned char get_final_diplomacy_score(void);
static unsigned char get_final_speed_score(void);

static unsigned char get_resource_base_price(unsigned char resource) {
    static const unsigned char base_prices[] = {
        6U,  /* timber */
        6U,  /* wool */
        8U,  /* iron */
        8U,  /* coal */
        12U, /* lumber */
        12U, /* fabric */
        16U, /* steel */
        20U, /* furniture */
        20U, /* clothes */
        24U, /* tools */
        28U  /* guns */
    };

    if (resource > RESOURCE_GUNS) {
        return 1;
    }

    return base_prices[resource];
}

unsigned char get_relation_tier(unsigned char relations) {
    if (relations < RELATION_POOR) {
        return 0U;
    }

    if (relations < RELATION_FAIR) {
        return 1U;
    }

    if (relations < RELATION_GOOD) {
        return 2U;
    }

    if (relations < RELATION_GREAT) {
        return 3U;
    }

    return 4U;
}

static unsigned int apply_percent(unsigned int value, unsigned char percent) {
    return ((value * percent) + 50U) / 100U;
}

static void add_resource_saturating(unsigned char resource, unsigned char amount) {
    unsigned int current;

    current = state.resources[resource];
    if ((unsigned int)(MAX_UINT - current) < amount) {
        state.resources[resource] = MAX_UINT;
        return;
    }

    state.resources[resource] = (unsigned int)(current + amount);
}

static void update_foreign_nation_prices(ForeignNation* nation) {
    unsigned char j;
    unsigned char relation_tier;

    relation_tier = get_relation_tier(nation->relations);
    for (j = 0; j < FOREIGN_TRADE_ENTRY_COUNT; ++j) {
        unsigned char export_base;
        unsigned char import_base;
        unsigned char export_percent;
        unsigned char import_percent;

        export_base = get_resource_base_price(nation->exports[j]);
        import_base = get_resource_base_price(nation->imports[j]);

        export_percent = FOREIGN_EXPORT_PRICE_BASE_PERCENT
                       + rand_range(0U, FOREIGN_EXPORT_PRICE_VARIANCE_PERCENT);
        if (export_percent > (relation_tier * FOREIGN_EXPORT_PRICE_RELATION_STEP_PERCENT)) {
            export_percent -= relation_tier * FOREIGN_EXPORT_PRICE_RELATION_STEP_PERCENT;
        }

        import_percent = FOREIGN_IMPORT_PRICE_BASE_PERCENT
                       + rand_range(0U, FOREIGN_IMPORT_PRICE_VARIANCE_PERCENT)
                       + (relation_tier * FOREIGN_IMPORT_PRICE_RELATION_STEP_PERCENT);

        nation->export_prices[j] = apply_percent(export_base, export_percent);
        nation->import_prices[j] = apply_percent(import_base, import_percent);
    }
}

static unsigned char rand_resource_excluding(unsigned char min, unsigned char max, unsigned char exclude) {
    unsigned char resource;

    do {
        resource = rand_range(min, max);
    } while (resource == exclude);

    return resource;
}

static unsigned char strings_equal(const char* left, const char* right) {
    while (*left == *right) {
        if (*left == '\0') {
            return TRUE;
        }
        ++left;
        ++right;
    }

    return FALSE;
}

static char* append_uint_decimal(char* buffer, unsigned int value) {
    unsigned int divisor;

    divisor = 1U;
    while ((value / divisor) >= 10U) {
        divisor *= 10UL;
    }

    do {
        *buffer++ = (char)('0' + (unsigned char)(value / divisor));
        value %= divisor;
        divisor /= 10U;
    } while (divisor != 0U);

    return buffer;
}

static unsigned char get_final_friendly_provinces(void) {
    unsigned char nation_index;
    unsigned char friendly_provinces;

    friendly_provinces = COUNCIL_MAJOR_POWER_PROVINCES;
    for (nation_index = 0U; nation_index < FOREIGN_NATION_COUNT; ++nation_index) {
        if (state.foreign_nations[nation_index].relations == RELATION_ALLY_COLONY) {
            if (nation_index < 2U) {
                friendly_provinces = (unsigned char)(friendly_provinces + COUNCIL_MAJOR_POWER_PROVINCES);
            } else {
                friendly_provinces = (unsigned char)(friendly_provinces + COUNCIL_MINOR_POWER_PROVINCES);
            }
        }
    }

    return friendly_provinces;
}

static unsigned char get_final_diplomacy_score(void) {
    unsigned char friendly_provinces;

    friendly_provinces = get_final_friendly_provinces();
    if (friendly_provinces <= COUNCIL_VICTORY_VOTES) {
        return 0U;
    }

    return (unsigned char)(((friendly_provinces - COUNCIL_VICTORY_VOTES) * 100U)
                         / (COUNCIL_TOTAL_PROVINCES - COUNCIL_VICTORY_VOTES));
}

static unsigned char get_final_treasury_score(void) {
    if (state.money >= SCORE_TREASURY_5) {
        return 100U;
    }
    if (state.money >= SCORE_TREASURY_4) {
        return 85U;
    }
    if (state.money >= SCORE_TREASURY_3) {
        return 65U;
    }
    if (state.money >= SCORE_TREASURY_2) {
        return 45U;
    }
    if (state.money >= SCORE_TREASURY_1) {
        return 25U;
    }

    return 10U;
}

static unsigned char get_final_speed_score(void) {
    if (state.turn_number <= SCORE_SPEED_TURN_1) {
        return SCORE_SPEED_SCORE_1;
    }
    if (state.turn_number <= SCORE_SPEED_TURN_2) {
        return SCORE_SPEED_SCORE_2;
    }
    if (state.turn_number <= SCORE_SPEED_TURN_3) {
        return SCORE_SPEED_SCORE_3;
    }
    if (state.turn_number <= SCORE_SPEED_TURN_4) {
        return SCORE_SPEED_SCORE_4;
    }
    if (state.turn_number <= SCORE_SPEED_TURN_5) {
        return SCORE_SPEED_SCORE_5;
    }

    return SCORE_SPEED_SCORE_6;
}

static unsigned int get_final_score(void) {
    unsigned int weighted_score;

    weighted_score = (unsigned int)(get_final_diplomacy_score() * SCORE_WEIGHT_DIPLOMACY)
                   + (unsigned int)(get_final_treasury_score() * SCORE_WEIGHT_TREASURY)
                   + (unsigned int)(get_final_speed_score() * SCORE_WEIGHT_SPEED);
    weighted_score /= 100U;
    return weighted_score * SCORE_SCALE_FACTOR;
}

void build_final_score_line(char* buffer) {
    static const char prefix[] = "Your score is ";
    const char* src;

    src = prefix;
    while (*src != '\0') {
        *buffer++ = *src++;
    }

    buffer = append_uint_decimal(buffer, get_final_score());
    *buffer = '\0';
}

unsigned char get_final_rank_index(void) {
    unsigned int score;

    score = get_final_score();
    if (score >= SCORE_RANK_THRESHOLD_VICTORIA) {
        return SCORE_RANK_VICTORIA;
    }
    if (score >= SCORE_RANK_THRESHOLD_BISMARCK) {
        return SCORE_RANK_BISMARCK;
    }
    if (score >= SCORE_RANK_THRESHOLD_NAPOLEON) {
        return SCORE_RANK_NAPOLEON;
    }
    if (score >= SCORE_RANK_THRESHOLD_CHARLES) {
        return SCORE_RANK_CHARLES;
    }

    return SCORE_RANK_FERDINAND;
}

void copy_text_limited(char* dest, const char* src, unsigned char capacity) {
    if (capacity == 0U) {
        return;
    }

    while (--capacity != 0U && *src != '\0') {
        *dest++ = *src++;
    }

    *dest = '\0';
}

static void assign_foreign_nation_names(void) {
    static const char great_power_name_pool[][8] = {
        "Deneb",
        "Haxaco",
        "Patagon",
        "Zimm",
        "Kem",
        "Ordune",
        "Devron"
    };
    static const char minor_nation_name_pool[][8] = {
        "Loke",
        "Pont",
        "Kathay",
        "Kessel",
        "Idolon",
        "Zazi",
        "Twelt", 
        "Manx",
        "Dedge", 
        "Sindel",
        "Wodan",
        "Bruhr",
        "Pram",
        "Issa"
    };
    unsigned char selected_great_power_count;
    unsigned char selected_minor_nation_count;
    unsigned char i;

    selected_great_power_count = 0U;
    while (selected_great_power_count < 2U) {
        unsigned char pool_index;
        unsigned char already_selected;
        unsigned char j;

        pool_index = rand_range(0U, (unsigned char)(sizeof(great_power_name_pool) / sizeof(great_power_name_pool[0])) - 1U);
        if (strings_equal(great_power_name_pool[pool_index], state.nation_name)) {
            continue;
        }

        already_selected = FALSE;
        for (j = 0; j < selected_great_power_count; ++j) {
            if (strings_equal(state.foreign_nations[j].name, great_power_name_pool[pool_index])) {
                already_selected = TRUE;
                break;
            }
        }

        if (already_selected) {
            continue;
        }

        copy_text_limited(state.foreign_nations[selected_great_power_count].name,
                          great_power_name_pool[pool_index],
                          FOREIGN_NATION_NAME_LENGTH + 1U);
        ++selected_great_power_count;
    }

    selected_minor_nation_count = 0U;
    while (selected_minor_nation_count < 3U) {
        unsigned char foreign_nation_index;
        unsigned char pool_index;
        unsigned char already_selected;
        unsigned char j;

        foreign_nation_index = (unsigned char)(selected_minor_nation_count + 2U);
        pool_index = rand_range(0U, (unsigned char)(sizeof(minor_nation_name_pool) / sizeof(minor_nation_name_pool[0])) - 1U);
        if (strings_equal(minor_nation_name_pool[pool_index], state.nation_name)) {
            continue;
        }

        already_selected = FALSE;
        for (j = 0; j < selected_minor_nation_count; ++j) {
            if (strings_equal(state.foreign_nations[j + 2U].name, minor_nation_name_pool[pool_index])) {
                already_selected = TRUE;
                break;
            }
        }

        if (already_selected) {
            continue;
        }

        copy_text_limited(state.foreign_nations[foreign_nation_index].name,
                          minor_nation_name_pool[pool_index],
                          FOREIGN_NATION_NAME_LENGTH + 1U);
        ++selected_minor_nation_count;
    }

    for (i = 0; i < FOREIGN_NATION_COUNT; ++i) {
        state.foreign_nations[i].name[FOREIGN_NATION_NAME_LENGTH] = '\0';
    }
}

void init_game() {
    unsigned char i;

    // Initialize game state with default values
    state.resources[RESOURCE_TIMBER] = 10;
    state.resources[RESOURCE_WOOL] = 10;
    state.resources[RESOURCE_IRON] = 5;
    state.resources[RESOURCE_COAL] = 5;

    state.resources[RESOURCE_LUMBER] = 2;
    state.resources[RESOURCE_FABRIC] = 2;
    state.resources[RESOURCE_STEEL] = 2;

    state.resources[RESOURCE_FURNITURE] = 2;
    state.resources[RESOURCE_CLOTHES] = 2;
    state.resources[RESOURCE_TOOLS] = 2;
    state.resources[RESOURCE_GUNS] = 2;

    state.number_of_provinces = 4;
    state.timber_yield_per_province = 2;
    state.wool_yield_per_province = 2;
    state.iron_yield_per_province = 1;
    state.coal_yield_per_province = 1;

    state.transport_timber = 5;
    state.transport_wool = 5;
    state.transport_iron = 1;
    state.transport_coal = 1;
    state.available_wagons = 5;

    state.production_lumber = 2;
    state.production_fabric = 2;
    state.production_steel = 1;

    state.production_furniture = 1;
    state.production_clothes = 1;
    state.production_tools = 0;
    state.production_guns = 0;

    state.available_workers = 6;

    state.traders = 2;
    state.warships = 2;
    state.capacity_per_trader = CAPACITY_PER_TRADER_BASE;
    state.guns_per_warship = GUNS_PER_WARSHIP_BASE;
    state.money = 250UL;
    state.science_level = 0U;

    assign_foreign_nation_names();

    for (i = 0; i < FOREIGN_NATION_COUNT; ++i) {
        unsigned char import_mid;
        ForeignNation* nation;

        nation = &state.foreign_nations[i];
        if (i < 2U) {
            nation->imports[0] = rand_range(RESOURCE_TIMBER, RESOURCE_COAL);
            nation->imports[1] = rand_resource_excluding(RESOURCE_TIMBER, RESOURCE_COAL,
                                                         nation->imports[0]);
            import_mid = rand_range(RESOURCE_LUMBER, RESOURCE_STEEL);
            nation->imports[2] = import_mid;

            nation->exports[0] = rand_range(RESOURCE_FURNITURE, RESOURCE_GUNS);
            nation->exports[1] = rand_resource_excluding(RESOURCE_FURNITURE, RESOURCE_GUNS,
                                                         nation->exports[0]);
            nation->exports[2] = rand_resource_excluding(RESOURCE_LUMBER, RESOURCE_STEEL, import_mid);
        } else {
            nation->imports[0] = rand_range(RESOURCE_FURNITURE, RESOURCE_GUNS);
            nation->imports[1] = rand_resource_excluding(RESOURCE_FURNITURE, RESOURCE_GUNS,
                                                         nation->imports[0]);
            import_mid = rand_range(RESOURCE_LUMBER, RESOURCE_STEEL);
            nation->imports[2] = import_mid;

            nation->exports[0] = rand_range(RESOURCE_TIMBER, RESOURCE_COAL);
            nation->exports[1] = rand_resource_excluding(RESOURCE_TIMBER, RESOURCE_COAL,
                                                         nation->exports[0]);
            nation->exports[2] = rand_resource_excluding(RESOURCE_LUMBER, RESOURCE_STEEL, import_mid);
        }

        nation->relations = RELATION_FAIR;
        nation->relations_previous_turn = nation->relations;
        update_foreign_nation_prices(nation);
    }

    state.trade_expenses = 0;
    state.trade_revenue = 0;
    state.turn_booty = 0;

    state.turn_number = 1;
    state.current_screen = SCREEN_MAIN;
    
    state.remaining_turn_capacity = state.traders * state.capacity_per_trader;
}

void next_turn() {
    unsigned char produced;
    unsigned char i;
    unsigned int upkeep = ((unsigned int)state.available_workers * UPKEEP_COST_PER_WORKER)
                        + ((unsigned int)state.traders * UPKEEP_COST_PER_TRADER)
                        + ((unsigned int)state.warships * UPKEEP_COST_PER_WARSHIP);

    // Update resources based on transport orders
    add_resource_saturating(RESOURCE_TIMBER, state.transport_timber);
    add_resource_saturating(RESOURCE_WOOL, state.transport_wool);
    add_resource_saturating(RESOURCE_IRON, state.transport_iron);
    add_resource_saturating(RESOURCE_COAL, state.transport_coal);

    // Apply production in sequence, limited by currently available inputs.
    produced = MIN(state.production_lumber, state.resources[RESOURCE_TIMBER] / 2U);
    state.resources[RESOURCE_TIMBER] -= produced * 2U;
    add_resource_saturating(RESOURCE_LUMBER, produced);

    produced = MIN(state.production_fabric, state.resources[RESOURCE_WOOL] / 2U);
    state.resources[RESOURCE_WOOL] -= produced * 2U;
    add_resource_saturating(RESOURCE_FABRIC, produced);

    produced = MIN(state.production_steel,
                   MIN(state.resources[RESOURCE_IRON] / 2U,
                       state.resources[RESOURCE_COAL] / 2U));
    state.resources[RESOURCE_IRON] -= produced * 2U;
    state.resources[RESOURCE_COAL] -= produced * 2U;
    add_resource_saturating(RESOURCE_STEEL, produced);

    produced = MIN(state.production_furniture, state.resources[RESOURCE_LUMBER] / 2U);
    state.resources[RESOURCE_LUMBER] -= produced * 2U;
    add_resource_saturating(RESOURCE_FURNITURE, produced);

    produced = MIN(state.production_clothes, state.resources[RESOURCE_FABRIC] / 2U);
    state.resources[RESOURCE_FABRIC] -= produced * 2U;
    add_resource_saturating(RESOURCE_CLOTHES, produced);

    produced = MIN(state.production_tools, state.resources[RESOURCE_STEEL] / 2U);
    state.resources[RESOURCE_STEEL] -= produced * 2U;
    add_resource_saturating(RESOURCE_TOOLS, produced);

    produced = MIN(state.production_guns, state.resources[RESOURCE_STEEL] / 2U);
    state.resources[RESOURCE_STEEL] -= produced * 2U;
    add_resource_saturating(RESOURCE_GUNS, produced);

    // Clamp persisted orders to what remains affordable for the next turn.
    state.production_lumber = MIN(state.production_lumber, state.resources[RESOURCE_TIMBER] / 2U);
    state.production_fabric = MIN(state.production_fabric, state.resources[RESOURCE_WOOL] / 2U);
    state.production_steel = MIN(state.production_steel, MIN(state.resources[RESOURCE_IRON] / 2U, state.resources[RESOURCE_COAL] / 2U));
    state.production_furniture = MIN(state.production_furniture, state.resources[RESOURCE_LUMBER] / 2U);
    state.production_clothes = MIN(state.production_clothes, state.resources[RESOURCE_FABRIC] / 2U);
    state.production_tools = MIN(state.production_tools, state.resources[RESOURCE_STEEL] / 2U);
    state.production_guns = MIN(state.production_guns, state.resources[RESOURCE_STEEL] / 2U);

    // profit & loss
    if (state.money > upkeep) {
        state.money -= upkeep;
    } else {
        state.money = 0UL;
    }

    state.trade_expenses = 0;
    state.trade_revenue = 0;
    state.turn_booty = 0;

    state.remaining_turn_capacity = state.traders * state.capacity_per_trader;

    // Update prices, then decrease relations with all foreign nations (except allies/colonies).
    // if money is zero, relations drop to bad immediately, otherwise they drop by a fixed amount
    for (i = 0; i < FOREIGN_NATION_COUNT; ++i) {
        ForeignNation* nation;

        nation = &state.foreign_nations[i];
        update_foreign_nation_prices(nation);
        if (nation->relations != RELATION_ALLY_COLONY) {
            nation->relations_previous_turn = nation->relations;
            if (state.money == 0) {
                nation->relations = RELATION_BAD;
            } else if (nation->relations <= RELATIONS_LOSS_PER_TURN) {
                nation->relations = RELATION_BAD;
            } else {
                nation->relations = (unsigned char)(nation->relations - RELATIONS_LOSS_PER_TURN);
            }
        }
    }

    // update turn number
    if (state.turn_number < MAX_UINT) {
        ++state.turn_number;
    }

    if ((state.turn_number % 10U) == 0U) {
        state.current_screen = SCREEN_COUNCIL_NATIONS;
    } else {
        state.current_screen = SCREEN_MAIN;
    }
}

 #pragma code-name (pop)
