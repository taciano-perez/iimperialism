#include <conio.h>
#include <stdio.h>
#include "game.h"
#include "strings.h"
#include "ui_buffers.h"

static void change_resource_production_order(const char* resource_name, unsigned char* production_order, unsigned int max_production) {
    signed int delta;
    unsigned char old_order;
    unsigned int new_production_order;
    while (1) {
        old_order = *production_order;
        clear_input_area();
        print(5, 20, "Produce how many units of");
        print(31, 20, resource_name);
        print(5, 21, STR_PER_TURN_MAX_FMT);
        sprintf(ui_buffer, STR_MAX_FMT, max_production);
        print(14, 21, ui_buffer);
        new_production_order = scan_uint(25, 21, 5);
        if (new_production_order > max_production) {
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
        print (5, 22, "or Quit?");
        key = cgetc();
        switch (key) {
            case 'l':
            case 'L':
                change_resource_production_order("lumber", &state.production_lumber, 
                    MIN(state.resources[RESOURCE_TIMBER] / 2, (state.production_lumber + state.available_workers)));
                return;
            case 'f':
            case 'F':
                change_resource_production_order("fabric", &state.production_fabric, 
                    MIN(state.resources[RESOURCE_WOOL] / 2, (state.production_fabric + state.available_workers)));
                return;
            case 's':
            case 'S':
                change_resource_production_order("steel", &state.production_steel, 
                    MIN(MIN(state.resources[RESOURCE_IRON] / 2, state.resources[RESOURCE_COAL] / 2), (state.production_steel + state.available_workers)));
                return;
            case 'U':
            case 'u':
                change_resource_production_order("furniture", &state.production_furniture,
                    MIN(state.resources[RESOURCE_LUMBER] / 2, (state.production_furniture + state.available_workers)));
                return;
            case 'c':
            case 'C':
                change_resource_production_order("clothes", &state.production_clothes,
                    MIN(state.resources[RESOURCE_FABRIC] / 2, (state.production_clothes + state.available_workers)));
                return;
            case 't':
            case 'T':
                change_resource_production_order("tools", &state.production_tools,
                    MIN(state.resources[RESOURCE_STEEL] / 2, (state.production_tools + state.available_workers)));
                return;
            case 'g':
            case 'G':
                change_resource_production_order("guns", &state.production_guns,
                    MIN(state.resources[RESOURCE_STEEL] / 2, (state.production_guns + state.available_workers)));
                return;
            case 'q':
            case 'Q':
                return;
        }
    }
}

static void train_new_workers(void) {
    unsigned int max_workers;
    unsigned int workers_to_train;

    clear_input_area();
    max_workers = MIN(state.resources[RESOURCE_FURNITURE], state.resources[RESOURCE_CLOTHES]);
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
            sprintf(ui_buffer, STR_MAX_FMT, max_workers);
            print (5, 22, ui_buffer);
            workers_to_train = scan_uint(12, 22, 5);
            if (workers_to_train > max_workers) {
                clear_input_area();
                continue;
            } else {
                state.resources[RESOURCE_FURNITURE] -= workers_to_train;
                state.resources[RESOURCE_CLOTHES] -= workers_to_train;
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
        case 'q':
        case 'Q':
            state.current_screen = SCREEN_INDUSTRY;
            break;
    }
}
