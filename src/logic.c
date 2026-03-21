#include "game.h"
#include <stdio.h>

#pragma code-name (push, "LOWCODE")

static unsigned char get_resource_base_price(unsigned char resource);
static unsigned char get_relation_tier(unsigned char relations);
static unsigned int apply_percent(unsigned int value, unsigned char percent);
static void update_foreign_market_prices(void);

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

void init_game() {
    static const char* foreign_nation_names[FOREIGN_NATION_COUNT] = {
        "Ordune",
        "Deneb",
        "Loke",
        "Pont",
        "Kathay"
    };
    static const unsigned char foreign_nation_relations[FOREIGN_NATION_COUNT] = {
        RELATION_NEUTRAL, RELATION_BAD, RELATION_GOOD, RELATION_EXCELLENT, RELATION_TERRIBLE
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

    state.traders = 2;
    state.frigates = 10;
    state.capacity_per_trader = CAPACITY_PER_TRADER_BASE;
    state.guns_per_frigate = GUNS_PER_FRIGATE_BASE;
    state.money = 2210000U;
    state.science_level = 0;

    for (i = 0; i < FOREIGN_NATION_COUNT; ++i) {
        snprintf(state.foreign_nations[i].name,
                 sizeof(state.foreign_nations[i].name),
                 "%s",
                 foreign_nation_names[i]);
        state.foreign_nations[i].relations = foreign_nation_relations[i];

        for (j = 0; j < FOREIGN_TRADE_ENTRY_COUNT; ++j) {
            state.foreign_nations[i].exports[j] = foreign_nation_exports[i][j];
            state.foreign_nations[i].imports[j] = foreign_nation_imports[i][j];
        }
    }

    update_foreign_market_prices();

    state.turn_number = 100;
    snprintf(state.nation_name, sizeof(state.nation_name), "Haxaco");
    state.current_screen = SCREEN_INDUSTRY;
    
    state.remaining_turn_capacity = state.traders * state.capacity_per_trader;
}

void next_turn() {
    // Update resources based on transport orders
    state.resources[RESOURCE_TIMBER] += state.transport_timber;
    state.resources[RESOURCE_WOOL] += state.transport_wool;
    state.resources[RESOURCE_IRON] += state.transport_iron;
    state.resources[RESOURCE_COAL] += state.transport_coal;

    // Update resources based on production orders 
    state.resources[RESOURCE_LUMBER] += state.production_lumber;
    state.resources[RESOURCE_TIMBER] -= 2 * state.production_lumber;
    state.resources[RESOURCE_FABRIC] += state.production_fabric;
    state.resources[RESOURCE_WOOL] -= 2 * state.production_fabric;
    state.resources[RESOURCE_STEEL] += state.production_steel;
    state.resources[RESOURCE_IRON] -= 2 * state.production_steel;
    state.resources[RESOURCE_COAL] -= 2 * state.production_steel;
    state.resources[RESOURCE_FURNITURE] += state.production_furniture;
    state.resources[RESOURCE_LUMBER] -= 2 * state.production_furniture;
    state.resources[RESOURCE_CLOTHES] += state.production_clothes;
    state.resources[RESOURCE_FABRIC] -= 2 * state.production_clothes;
    state.resources[RESOURCE_TOOLS] += state.production_tools;
    state.resources[RESOURCE_STEEL] -= 2 * state.production_tools;
    state.resources[RESOURCE_GUNS] += state.production_guns;
    state.resources[RESOURCE_STEEL] -= 2 * state.production_guns;

    // cap production orders to available resources
    state.production_lumber = MIN(state.production_lumber, state.resources[RESOURCE_TIMBER] / 2);
    state.production_fabric = MIN(state.production_fabric, state.resources[RESOURCE_WOOL] / 2);
    state.production_steel = MIN(state.production_steel, MIN(state.resources[RESOURCE_IRON] / 2, state.resources[RESOURCE_COAL] / 2));
    state.production_furniture = MIN(state.production_furniture, state.resources[RESOURCE_LUMBER] / 2);
    state.production_clothes = MIN(state.production_clothes, state.resources[RESOURCE_FABRIC] / 2);
    state.production_tools = MIN(state.production_tools, state.resources[RESOURCE_STEEL] / 2);
    state.production_guns = MIN(state.production_guns, state.resources[RESOURCE_STEEL] / 2);

    update_foreign_market_prices();

    state.remaining_turn_capacity = state.traders * state.capacity_per_trader;

    // update turn number
    ++state.turn_number;
}

 #pragma code-name (pop)
