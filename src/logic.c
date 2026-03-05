#include "game.h"
#include <stdio.h>

#pragma code-name (push, "LOWCODE")

void init_game() {
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
