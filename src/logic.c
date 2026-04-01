#include "game.h"
#include <string.h>

#pragma code-name (push, "LOWCODE")

static unsigned char get_resource_base_price(unsigned char resource);
static unsigned char get_relation_tier(unsigned char relations);
static unsigned int apply_percent(unsigned int value, unsigned char percent);
static void add_resource_saturating(unsigned char resource, unsigned char amount);
static void update_foreign_market_prices(void);
static void assign_foreign_nation_names(void);

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

static unsigned char get_relation_tier(unsigned char relations) {
    if (relations < RELATION_BAD) {
        return 0U;
    }

    if (relations < RELATION_NEUTRAL) {
        return 1U;
    }

    if (relations < RELATION_GOOD) {
        return 2U;
    }

    if (relations < RELATION_EXCELLENT) {
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

static void update_foreign_market_prices(void) {
    unsigned char i;
    unsigned char j;

    for (i = 0; i < FOREIGN_NATION_COUNT; ++i) {
        unsigned char relation_tier;

        relation_tier = get_relation_tier(state.foreign_nations[i].relations);

        for (j = 0; j < FOREIGN_TRADE_ENTRY_COUNT; ++j) {
            unsigned char export_base;
            unsigned char import_base;
            unsigned char export_percent;
            unsigned char import_percent;

            export_base = get_resource_base_price(state.foreign_nations[i].exports[j]);
            import_base = get_resource_base_price(state.foreign_nations[i].imports[j]);

            export_percent = 85U + rand_range(0U, 15U);
            if (export_percent > (relation_tier * 3U)) {
                export_percent -= relation_tier * 3U;
            }

            import_percent = 110U + rand_range(0U, 20U) + (relation_tier * 4U);

            state.foreign_nations[i].export_prices[j] = apply_percent(export_base, export_percent);
            state.foreign_nations[i].import_prices[j] = apply_percent(import_base, import_percent);
        }
    }
}

static void assign_foreign_nation_names(void) {
    static const char* great_power_name_pool[] = {
        "Deneb",
        "Haxaco",
        "Patagon",
        "Zimm",
        "Kem",
        "Ordune",
        "Devron"
    };
    static const char* minor_nation_name_pool[] = {
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
        if (strcmp(great_power_name_pool[pool_index], state.nation_name) == 0) {
            continue;
        }

        already_selected = FALSE;
        for (j = 0; j < selected_great_power_count; ++j) {
            if (strcmp(state.foreign_nations[j].name, great_power_name_pool[pool_index]) == 0) {
                already_selected = TRUE;
                break;
            }
        }

        if (already_selected) {
            continue;
        }

        strcpy(state.foreign_nations[selected_great_power_count].name, great_power_name_pool[pool_index]);
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
        if (strcmp(minor_nation_name_pool[pool_index], state.nation_name) == 0) {
            continue;
        }

        already_selected = FALSE;
        for (j = 0; j < selected_minor_nation_count; ++j) {
            if (strcmp(state.foreign_nations[j + 2U].name, minor_nation_name_pool[pool_index]) == 0) {
                already_selected = TRUE;
                break;
            }
        }

        if (already_selected) {
            continue;
        }

        strcpy(state.foreign_nations[foreign_nation_index].name, minor_nation_name_pool[pool_index]);
        ++selected_minor_nation_count;
    }

    for (i = 0; i < FOREIGN_NATION_COUNT; ++i) {
        state.foreign_nations[i].name[FOREIGN_NATION_NAME_LENGTH] = '\0';
    }
}

void init_game() {
    static const unsigned char foreign_nation_relations[FOREIGN_NATION_COUNT] = {
        RELATION_EXCELLENT, RELATION_EXCELLENT, RELATION_EXCELLENT, RELATION_EXCELLENT, RELATION_EXCELLENT
    };
    static const unsigned char foreign_nation_exports[FOREIGN_NATION_COUNT][FOREIGN_TRADE_ENTRY_COUNT] = {
        { RESOURCE_STEEL, RESOURCE_FURNITURE, RESOURCE_LUMBER },
        { RESOURCE_CLOTHES, RESOURCE_TOOLS, RESOURCE_GUNS },
        { RESOURCE_WOOL, RESOURCE_WOOL, RESOURCE_COAL },
        { RESOURCE_IRON, RESOURCE_TIMBER, RESOURCE_WOOL },
        { RESOURCE_WOOL, RESOURCE_COAL, RESOURCE_TIMBER }
    };
    static const unsigned char foreign_nation_imports[FOREIGN_NATION_COUNT][FOREIGN_TRADE_ENTRY_COUNT] = {
        { RESOURCE_TIMBER, RESOURCE_IRON, RESOURCE_WOOL },
        { RESOURCE_COAL, RESOURCE_TIMBER, RESOURCE_WOOL },
        { RESOURCE_STEEL, RESOURCE_GUNS, RESOURCE_CLOTHES },
        { RESOURCE_FURNITURE, RESOURCE_CLOTHES, RESOURCE_GUNS },
        { RESOURCE_FURNITURE, RESOURCE_GUNS, RESOURCE_LUMBER }
    };
    unsigned char i;
    unsigned char j;

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

    state.traders = 8;
    state.frigates = 24;
    state.capacity_per_trader = CAPACITY_PER_TRADER_BASE;
    state.guns_per_frigate = GUNS_PER_FRIGATE_BASE;
    state.money = 30000U;
    state.science_level = 0;

    assign_foreign_nation_names();

    for (i = 0; i < FOREIGN_NATION_COUNT; ++i) {
        state.foreign_nations[i].relations = foreign_nation_relations[i];
        state.foreign_nations[i].relations_previous_turn = foreign_nation_relations[i];

        for (j = 0; j < FOREIGN_TRADE_ENTRY_COUNT; ++j) {
            state.foreign_nations[i].exports[j] = foreign_nation_exports[i][j];
            state.foreign_nations[i].imports[j] = foreign_nation_imports[i][j];
        }
    }

    update_foreign_market_prices();

    state.turn_number = 99;
    state.current_screen = SCREEN_INDUSTRY;
    
    state.remaining_turn_capacity = state.traders * state.capacity_per_trader;
}

void next_turn() {
    unsigned char produced;
    unsigned char i;

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

    update_foreign_market_prices();

    state.remaining_turn_capacity = state.traders * state.capacity_per_trader;

    // decrease relations with all foreign nations (except allies/colonies)
    for (i = 0; i < FOREIGN_NATION_COUNT; ++i) {
        if (state.foreign_nations[i].relations != RELATION_ALLY_COLONY) {
            state.foreign_nations[i].relations = MAX(0, state.foreign_nations[i].relations - RELATIONS_LOSS_PER_TURN);
        }
    }

    // update turn number
    ++state.turn_number;

    if ((state.turn_number % 10U) == 0U) {
        state.current_screen = SCREEN_COUNCIL_NATIONS;
    } else {
        state.current_screen = SCREEN_INDUSTRY;
    }
}

 #pragma code-name (pop)
