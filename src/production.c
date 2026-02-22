#include <conio.h>
#include <stdio.h>
#include "game.h"
#include "strings.h"

static char buffer[42];

static void change_resource_production_order(const char* resource_name, unsigned int* production_order, unsigned int max_production) {
    signed int delta;
    unsigned int old_order;
    unsigned int new_production_order;
    while (1) {
        old_order = *production_order;
        clear_input_area();
        sprintf(buffer, "Produce how many units of %s", resource_name);
        print(5, 20, buffer);
        sprintf(buffer, STR_PER_TURN_MAX_FMT, max_production);
        print(5, 21, buffer);
        new_production_order = scan_uint(25, 21, 5);
        if (new_production_order > max_production) {
            print(5, 22, STR_SIR_WE_LACK_RESOURCES);
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

static void production_orders(void) {
    char key;
    while (1) {
        clear_input_area();
        print (5, 20, "Orders for Lumber, Fabric, Steel, ");
        print (5, 21, "fUrniture, Clothes, Tools, Guns,");
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
            case 'g':
            case 'G':
                change_resource_production_order("guns", &state.production_guns,
                    MIN(state.steel / 2, (state.production_guns + state.available_workers)));
                return;
            case 'r':
            case 'R':
                return;
            default:
                print(18, 22, STR_INVALID_ANSWER);
                cgetc();
                continue;
        }
    }
}

static void train_new_workers(void) {
    unsigned int max_workers;
    unsigned int workers_to_train;

    clear_input_area();
    max_workers = MIN(state.furniture, state.clothes);
    if (max_workers == 0) {
        print(5, 20, STR_SIR_TRAIN_WORKERS1);
        print(5, 21, STR_SIR_TRAIN_WORKERS2);
        print(20, 22, STR_NOT_ENOUGH_RESOURCES);
        cgetc();
        return;
    } else {
        while (1) {
            print(5, 20, STR_SIR_TRAIN_WORKERS1);
            print(5, 21, STR_SIR_TRAIN_WORKERS2);
            print(20, 21, "Train how many?");
            sprintf(buffer, STR_MAX_FMT, max_workers);
            print (5, 22, buffer);
            workers_to_train = scan_uint(12, 22, 5);
            if (workers_to_train > max_workers) {
                print(19, 22, STR_NOT_ENOUGH_RESOURCES);
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
