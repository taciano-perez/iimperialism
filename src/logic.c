#include "game.h"
#include <stdio.h>

#pragma code-name (push, "LOWCODE")

void init_game() {
    static const char* foreign_nation_names[FOREIGN_NATION_COUNT] = {
        "Ordune",
        "Deneb",
        "Loke",
        "Pont",
        "Kathay"
    };
    static const int foreign_nation_relations[FOREIGN_NATION_COUNT] = {
        RELATION_NEUTRAL, RELATION_BAD, RELATION_GOOD, RELATION_EXCELLENT, RELATION_TERRIBLE
    };
    static const int foreign_nation_exports[FOREIGN_NATION_COUNT][FOREIGN_TRADE_ENTRY_COUNT] = {
        { RESOURCE_STEEL, RESOURCE_FURNITURE, RESOURCE_LUMBER },
        { RESOURCE_CLOTHES, RESOURCE_TOOLS, RESOURCE_GUNS },
        { RESOURCE_WOOL, RESOURCE_WOOL, RESOURCE_COAL },
        { RESOURCE_IRON, RESOURCE_TIMBER, RESOURCE_WOOL },
        { RESOURCE_WOOL, RESOURCE_COAL, RESOURCE_TIMBER }
    };
    static const int foreign_nation_imports[FOREIGN_NATION_COUNT][FOREIGN_TRADE_ENTRY_COUNT] = {
        { RESOURCE_TIMBER, RESOURCE_IRON, RESOURCE_WOOL },
        { RESOURCE_COAL, RESOURCE_TIMBER, RESOURCE_WOOL },
        { RESOURCE_STEEL, RESOURCE_GUNS, RESOURCE_CLOTHES },
        { RESOURCE_FURNITURE, RESOURCE_CLOTHES, RESOURCE_GUNS },
        { RESOURCE_FURNITURE, RESOURCE_GUNS, RESOURCE_LUMBER }
    };
    unsigned char i;
    unsigned char j;

    // Initialize game state with default values
    state.timber = 10;
    state.wool = 10;
    state.iron = 5;
    state.coal = 5;

    state.lumber = 2;
    state.fabric = 2;
    state.steel = 2;

    state.furniture = 2;
    state.clothes = 2;
    state.tools = 2;
    state.guns = 2;

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
    state.frigates = 1;

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

    state.turn_number = 1;
    snprintf(state.nation_name, sizeof(state.nation_name), "Haxaco");
    state.current_screen = SCREEN_DIPLOMACY;
}

void next_turn() {
    // Update resources based on transport orders
    state.timber += state.transport_timber;
    state.wool += state.transport_wool;
    state.iron += state.transport_iron;
    state.coal += state.transport_coal;

    // Update resources based on production orders 
    state.lumber += state.production_lumber;
    state.timber -= 2 * state.production_lumber;
    state.fabric += state.production_fabric;
    state.wool -= 2 * state.production_fabric;
    state.steel += state.production_steel;
    state.iron -= 2 * state.production_steel;
    state.coal -= 2 * state.production_steel;
    state.furniture += state.production_furniture;
    state.lumber -= 2 * state.production_furniture;
    state.clothes += state.production_clothes;
    state.fabric -= 2 * state.production_clothes;
    state.tools += state.production_tools;
    state.steel -= 2 * state.production_tools;
    state.guns += state.production_guns;
    state.steel -= 2 * state.production_guns;

    // cap production orders to available resources
    state.production_lumber = MIN(state.production_lumber, state.timber / 2);
    state.production_fabric = MIN(state.production_fabric, state.wool / 2);
    state.production_steel = MIN(state.production_steel, MIN(state.iron / 2, state.coal / 2));
    state.production_furniture = MIN(state.production_furniture, state.lumber / 2);
    state.production_clothes = MIN(state.production_clothes, state.fabric / 2);
    state.production_tools = MIN(state.production_tools, state.steel / 2);
    state.production_guns = MIN(state.production_guns, state.steel / 2);

    // update turn number
    state.turn_number++;
}

 #pragma code-name (pop)
