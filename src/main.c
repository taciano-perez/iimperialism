#include <conio.h>
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "overlay.h"

#define BOX1_X1 0
#define BOX1_Y1 2
#define BOX1_X2 39
#define BOX1_Y2 7

// Global state instance
GameState state;

char buffer[42];

void render_warehouse_box() {
    box(BOX1_X1, BOX1_Y1, BOX1_X2, BOX1_Y2);
    print ((BOX1_X1+1), BOX1_Y1, "Warehouse");

    print((BOX1_X1+1), (BOX1_Y1+1), "Timber: ");
    print_int_right_aligned((BOX1_X1+11), (BOX1_Y1+1), state.timber);
    print((BOX1_X1+1), (BOX1_Y1+2), "Wool: ");
    print_int_right_aligned((BOX1_X1+11), (BOX1_Y1+2), state.wool);
    print((BOX1_X1+1), (BOX1_Y1+3), "Iron: ");
    print_int_right_aligned((BOX1_X1+11), (BOX1_Y1+3), state.iron);
    print((BOX1_X1+1), (BOX1_Y1+4), "Coal: ");
    print_int_right_aligned((BOX1_X1+11), (BOX1_Y1+4), state.coal);

    print((BOX1_X1+13), (BOX1_Y1+1), "Lumber: ");
    print_int_right_aligned((BOX1_X1+23), (BOX1_Y1+1), state.lumber);
    print((BOX1_X1+13), (BOX1_Y1+2), "Fabric: ");
    print_int_right_aligned((BOX1_X1+23), (BOX1_Y1+2), state.fabric);
    print((BOX1_X1+13), (BOX1_Y1+3), "Steel: ");
    print_int_right_aligned((BOX1_X1+23), (BOX1_Y1+3), state.steel);

    print((BOX1_X1+25), (BOX1_Y1+1), "Furniture: ");
    print_int_right_aligned((BOX1_X1+39), (BOX1_Y1+1), state.furniture);
    print((BOX1_X1+25), (BOX1_Y1+2), "Clothes: ");
    print_int_right_aligned((BOX1_X1+39), (BOX1_Y1+2), state.clothes);
    print((BOX1_X1+25), (BOX1_Y1+3), "Tools: ");
    print_int_right_aligned((BOX1_X1+39), (BOX1_Y1+3), state.tools);
    print((BOX1_X1+25), (BOX1_Y1+4), "Cannons: ");
    print_int_right_aligned((BOX1_X1+39), (BOX1_Y1+4), state.cannons);
}

void change_resource_production_order(const char* resource_name, unsigned int* production_order, unsigned int max_production) {
    signed int delta;
    unsigned int old_order;
    unsigned int new_production_order;
    while (1) {
        old_order = *production_order;
        clear_input_area();
        sprintf(buffer, "Produce how many units of %s", resource_name);
        print(5, 20, buffer);
        sprintf(buffer, "per turn? (max %u)", max_production);
        print(5, 21, buffer);
        new_production_order = scan_uint(25, 21, 5);
        if (new_production_order > max_production) {
            print(5, 22, "Sir, we lack resources!");
            cgetc();
            continue;
        } else {
            *production_order = new_production_order;
            delta = (signed int)*production_order - (signed int)old_order;
            state.available_workers -= delta;
            break;
        }
    }
}

void production_orders() {
    char key;
    while (1) {
        clear_input_area();
        print (5, 20, "Orders for Lumber, Fabric, Steel, ");
        print (5, 21, "fUrniture, Clothes, Tools, caNnons,");
        print (5, 22, "or Return?");
        key = cgetc_at(16, 22);
        switch (key) {
            case 'l':
            case 'L':
                change_resource_production_order("lumber", &state.production_lumber, 
                    MIN(state.timber / 2, (state.production_lumber + state.available_workers)));
                return;
            case 'f':
            case 'F':
                change_resource_production_order("fabric", &state.production_fabric, 
                    MIN(state.wool / 2, (state.production_fabric + state.available_workers)));
                return;
            case 's':
            case 'S':
                change_resource_production_order("steel", &state.production_steel, 
                    MIN(MIN(state.iron / 2, state.coal / 2), (state.production_steel + state.available_workers)));
                return;
            case 'U':
            case 'u':
                change_resource_production_order("furniture", &state.production_furniture,
                    MIN(state.lumber / 2, (state.production_furniture + state.available_workers)));
                return;
            case 'c':
            case 'C':
                change_resource_production_order("clothes", &state.production_clothes,
                    MIN(state.fabric / 2, (state.production_clothes + state.available_workers)));
                return;
            case 't':
            case 'T':
                change_resource_production_order("tools", &state.production_tools,
                    MIN(state.steel / 2, (state.production_tools + state.available_workers)));
                return;
            case 'n':
            case 'N':
                change_resource_production_order("cannons", &state.production_cannons,
                    MIN(state.steel / 2, (state.production_cannons + state.available_workers)));
                return;
            case 'r':
            case 'R':
                return;
            default:
                print(18, 22, "Invalid answer!");
                cgetc();
                continue;
        }
    }
}

void train_new_workers() {
    unsigned int max_workers;
    unsigned int workers_to_train;

    clear_input_area();
    max_workers = MIN(state.furniture, state.clothes);
    if (max_workers == 0) {
        print(5, 20, "Sir, a worker costs 1 furniture");
        print(5, 21, "and 1 clothes. We lack resources!");
        cgetc();
        return;
    } else {
        while (1) {
            print(5, 20, "Sir, A worker costs 1 furniture");
            print(5, 21, "and 1 clothes. Train how many?");
            sprintf(buffer, "(max %u)", max_workers);
            print (5, 22, buffer);
            workers_to_train = scan_uint(12, 22, 5);
            if (workers_to_train > max_workers) {
                print(19, 22, "Not enough resources!");
                cgetc();
                clear_input_area();
                continue;
            } else {
                state.furniture -= workers_to_train;
                state.clothes -= workers_to_train;
                state.available_workers += workers_to_train;
                break;
            }
        }
    }
}

void change_resource_transport_order(const char* resource_name, unsigned int* transport_order, unsigned int max_transport) {
    signed int delta;
    unsigned int old_order;
    while (1) {
        old_order = *transport_order;
        clear_input_area();
        sprintf(buffer, "Transport how many units of %s", resource_name);
        print(5, 20, buffer);
        sprintf(buffer, "per turn? (max %u)", max_transport);
        print(5, 21, buffer);
        *transport_order = scan_uint(25, 21, 5);
        if (*transport_order > max_transport) {
            print(5, 22, "Sir, we lack resources!");
            cgetc();
            continue;
        } else {
            delta = (signed int)*transport_order - (signed int)old_order;
            state.transport_timber = *transport_order;
            state.available_wagons -= delta;
            break;
        }
    }
}

void transport_orders() {
    char key;
    while (1) {
        clear_input_area();
        print (5, 20, "Orders for Timber, Wool, Iron,");
        print (5, 21, "or Coal?");
        key = cgetc_at(14, 21);
        switch (key) {
            case 't':
            case 'T':
                change_resource_transport_order("timber", &state.transport_timber,
                    MIN(state.timber_yield_per_province * state.number_of_provinces, state.transport_timber + state.available_wagons));
                return;
            case 'w':
            case 'W':
                change_resource_transport_order("wool", &state.transport_wool,
                    MIN(state.wool_yield_per_province * state.number_of_provinces, state.transport_wool + state.available_wagons));
                return;
            case 'i':
            case 'I':
                change_resource_transport_order("iron", &state.transport_iron,
                    MIN(state.iron_yield_per_province * state.number_of_provinces, state.transport_iron + state.available_wagons));
                return;
            case 'c':
            case 'C':
                change_resource_transport_order("coal", &state.transport_coal,
                    MIN(state.coal_yield_per_province * state.number_of_provinces, state.transport_coal + state.available_wagons));
                return;
            default:
                print(5, 22, "Invalid answer!");
                cgetc();
                continue;
        }
    }
}

void build_wagons() {
    unsigned int max_wagons;
    unsigned int wagons_to_build;

    clear_input_area();
    max_wagons = MIN(state.lumber, state.steel);
    if (max_wagons == 0) {
        print(5, 20, "Not enough resources!");
        cgetc();
        return;
    } else {
        while (1) {
            sprintf(buffer, "Build how many wagons? (max %u)", max_wagons);
            print (5, 20, buffer);
            wagons_to_build = scan_uint(5, 21, 5);
            if (wagons_to_build > max_wagons) {
                print(5, 22, "Not enough resources!");
                cgetc();
                clear_input_area();
                continue;
            } else {
                state.lumber -= wagons_to_build;
                state.steel -= wagons_to_build;
                state.available_wagons += wagons_to_build;
                break;
            }
        }
    }
}

void handle_screen_input_production(char key) {
    switch (key) {
        case 'p':
        case 'P':
            production_orders();
            break;
        case 't':
        case 'T':
            train_new_workers();
            break;
        case 'r':
        case 'R':
            state.current_screen = SCREEN_INDUSTRY;
            break;
    }
}

void handle_screen_input_transport(char key) {
    switch (key) {
        case 'b':
        case 'B':
            build_wagons();
            break;
        case 't':
        case 'T':
            transport_orders();
            break;
        case 'r':
        case 'R':
            state.current_screen = SCREEN_INDUSTRY;
            break;
    }
}

void handle_screen_input_industry(char key) {
    switch (key) {
        case 't':
        case 'T':
            state.current_screen = SCREEN_TRANSPORT;
            break;
        case 'p':
        case 'P':
            state.current_screen = SCREEN_PRODUCTION;
            break;
        case 'e':
        case 'E':
            next_turn();
            break;
    }
}

int main(void) {

    char key;

    init_game();
    init_overlays();
    ui_init();
    while (1) { // main game loop
        if (state.current_screen == SCREEN_INDUSTRY) {
            run_overlay(OVL_INDUSTRY);
        } else if (state.current_screen == SCREEN_TRANSPORT) {
            run_overlay(OVL_TRANSPORT);
        } else if (state.current_screen == SCREEN_PRODUCTION) {
            run_overlay(OVL_PRODUCTION);
        } else if (state.current_screen == SCREEN_ADMIRALTY) {
            run_overlay(OVL_ADMIRALTY);
        }

        key = cgetc();

        switch (key) {
            case 27: // ESC key
                print(0, 15, "Exiting game");
                ui_exit();
                return 0;
            default:
                if (state.current_screen == SCREEN_INDUSTRY) {
                    handle_screen_input_industry(key);
                } else  if (state.current_screen == SCREEN_TRANSPORT) {
                    handle_screen_input_transport(key);
                } else if (state.current_screen == SCREEN_PRODUCTION) {
                    handle_screen_input_production(key);
                } else if (state.current_screen == SCREEN_ADMIRALTY) {
                    // TODO: input handling for Admiralty screen
                }
                break;
        }
    }

}