#include <conio.h>
#include <stdio.h>
#include "game.h"
#include "strings.h"

static char buffer[6];

static void change_resource_transport_order(const char* resource_name, unsigned int* transport_order, unsigned int max_transport) {
    signed int delta;
    unsigned int old_order;
    while (1) {
        old_order = *transport_order;
        clear_input_area();
        sprintf(buffer, "Transport how many units of %s", resource_name);
        print(5, 20, buffer);
        sprintf(buffer, STR_PER_TURN_MAX_FMT, max_transport);
        print(5, 21, buffer);
        *transport_order = scan_uint(25, 21, 5);
        if (*transport_order > max_transport) {
            print(5, 22, STR_SIR_WE_LACK_RESOURCES);
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

static void transport_orders(void) {
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
                print(5, 22, STR_INVALID_ANSWER);
                cgetc();
                continue;
        }
    }
}

static void build_wagons(void) {
    unsigned int max_wagons;
    unsigned int wagons_to_build;

    clear_input_area();
    max_wagons = MIN(state.resources[RESOURCE_LUMBER], state.resources[RESOURCE_STEEL]);
    if (max_wagons == 0) {
        print(5, 20, STR_NOT_ENOUGH_RESOURCES);
        cgetc();
        return;
    } else {
        while (1) {
            sprintf(buffer, "Build how many wagons? (max %u)", max_wagons);
            print (5, 20, buffer);
            wagons_to_build = scan_uint(5, 21, 5);
            if (wagons_to_build > max_wagons) {
                print(5, 22, STR_NOT_ENOUGH_RESOURCES);
                cgetc();
                clear_input_area();
                continue;
            } else {
                state.resources[RESOURCE_LUMBER] -= wagons_to_build;
                state.resources[RESOURCE_STEEL] -= wagons_to_build;
                state.available_wagons += wagons_to_build;
                break;
            }
        }
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
