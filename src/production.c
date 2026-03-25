#include "game.h"

void production_orders(void) {
    char key;
    const char* resource_name;
    unsigned char* production_order;
    unsigned char old_order;
    signed int delta;
    unsigned int max_production;
    unsigned int new_production_order;

    while (1) {
        clear_input_area();
        print (5, 20, "Orders for Lumber, Fabric, Steel, ");
        print (5, 21, "fUrniture, Clothes, Tools, Guns,");
        print (5, 22, "or Quit?");
        key = cgetc_at(13, 22);
        switch (key) {
            case 'l':
            case 'L':
                resource_name = "lumber";
                production_order = &state.production_lumber;
                max_production = MIN(state.resources[RESOURCE_TIMBER] / 2, state.production_lumber + state.available_workers);
                break;
            case 'f':
            case 'F':
                resource_name = "fabric";
                production_order = &state.production_fabric;
                max_production = MIN(state.resources[RESOURCE_WOOL] / 2, state.production_fabric + state.available_workers);
                break;
            case 's':
            case 'S':
                resource_name = "steel";
                production_order = &state.production_steel;
                max_production = MIN(MIN(state.resources[RESOURCE_IRON] / 2, state.resources[RESOURCE_COAL] / 2),
                    state.production_steel + state.available_workers);
                break;
            case 'u':
            case 'U':
                resource_name = "furniture";
                production_order = &state.production_furniture;
                max_production = MIN(state.resources[RESOURCE_LUMBER] / 2, state.production_furniture + state.available_workers);
                break;
            case 'c':
            case 'C':
                resource_name = "clothes";
                production_order = &state.production_clothes;
                max_production = MIN(state.resources[RESOURCE_FABRIC] / 2, state.production_clothes + state.available_workers);
                break;
            case 't':
            case 'T':
                resource_name = "tools";
                production_order = &state.production_tools;
                max_production = MIN(state.resources[RESOURCE_STEEL] / 2, state.production_tools + state.available_workers);
                break;
            case 'g':
            case 'G':
                resource_name = "guns";
                production_order = &state.production_guns;
                max_production = MIN(state.resources[RESOURCE_STEEL] / 2, state.production_guns + state.available_workers);
                break;
            case 'q':
            case 'Q':
                return;
            default:
                continue;
        }

        while (1) {
            old_order = *production_order;
            clear_input_area();
            print(5, 20, "Produce how many units of");
            print(31, 20, resource_name);
            print(5, 21, "per turn (Max:    )?");
            print_int_right_aligned(22, 21, max_production);
            new_production_order = scan_uint(25, 21, 5);
            if (new_production_order > max_production) {
                continue;
            }

            *production_order = new_production_order;
            delta = (signed int)*production_order - (signed int)old_order;
            state.available_workers -= delta;
            return;
        }
    }
}
