#include "game.h"
#include "overlay.h"

#define BOX_X1 0
#define BOX_Y1 8
#define BOX_X2 39
#define BOX_Y2 13

void render_transport_screen(void) {
    while (1) {
        char key;
        const char* resource_name;
        unsigned char* transport_order;
        unsigned char old_order;
        signed int delta;
        unsigned int max_transport;
        unsigned int wagons_to_build;
        unsigned int new_transport_order;

        clear_screen();
        print(0, 0, "Transport Orders");
        render_turn_funds_header();
        render_warehouse_box();

    box(BOX_X1, BOX_Y1+1, BOX_X2, BOX_Y2);
    print_inverted(BOX_X1, BOX_Y1, "Transport Orders");
    print((BOX_X1+1), (BOX_Y1+1), "Timber: ");
    print_int_right_aligned((BOX_X1+11), (BOX_Y1+1), state.transport_timber);
    print((BOX_X1+13), (BOX_Y1+1), "/");
    print_int_right_aligned((BOX_X1+18), (BOX_Y1+1), state.timber_yield_per_province * state.number_of_provinces);
    print((BOX_X1+1), (BOX_Y1+2), "Wool: ");
    print_int_right_aligned((BOX_X1+11), (BOX_Y1+2), state.transport_wool);
    print((BOX_X1+13), (BOX_Y1+2), "/");
    print_int_right_aligned((BOX_X1+18), (BOX_Y1+2), state.wool_yield_per_province * state.number_of_provinces);
    print((BOX_X1+1), (BOX_Y1+3), "Iron: ");
    print_int_right_aligned((BOX_X1+11), (BOX_Y1+3), state.transport_iron);
    print((BOX_X1+13), (BOX_Y1+3), "/");
    print_int_right_aligned((BOX_X1+18), (BOX_Y1+3), state.iron_yield_per_province * state.number_of_provinces);
    print((BOX_X1+1), (BOX_Y1+4), "Coal: ");
    print_int_right_aligned((BOX_X1+11), (BOX_Y1+4), state.transport_coal);
    print((BOX_X1+13), (BOX_Y1+4), "/");
    print_int_right_aligned((BOX_X1+18), (BOX_Y1+4), state.coal_yield_per_province * state.number_of_provinces);

    print(1, 15, "Idle wagons: ");
    print_int_right_aligned(20, 15, state.available_wagons);
    print(1, 16, "Cost of new wagon: 1 lumber + 1 steel");

        draw_picture_at(INDUSTRY_PORTRAIT, 0, 20);
        print(5, 20, "What are your orders, sir?");
        print(5, 21, "Change Transport per turn,");
        print(5, 22, "build Wagons or Quit?");

        key = cgetc_at(26, 22);
        switch (key) {
            case 'w':
            case 'W':
                clear_input_area();
                max_transport = MIN(state.resources[RESOURCE_LUMBER], state.resources[RESOURCE_STEEL]);
                max_transport = MIN(max_transport, MAX_UCHAR - state.available_wagons);
                while (1) {
                    print(5, 20, "Build how many wagons (Max:    )?");
                    print_int_right_aligned(35, 20, max_transport);
                    wagons_to_build = scan_uint(5, 21, 5);
                    if (wagons_to_build > max_transport) {
                        clear_input_area();
                        continue;
                    }

                    state.resources[RESOURCE_LUMBER] -= wagons_to_build;
                    state.resources[RESOURCE_STEEL] -= wagons_to_build;
                    state.available_wagons += wagons_to_build;
                    break;
                }
                break;
            case 't':
            case 'T':
                while (1) {
                    clear_input_area();
                    print(5, 20, "Orders for Timber, Wool, Iron,");
                    print(5, 21, "or Coal?");
                    key = cgetc_at(13, 21);
                    switch (key) {
                        case 't':
                        case 'T':
                            resource_name = "timber";
                            transport_order = &state.transport_timber;
                            max_transport = MIN(state.timber_yield_per_province * state.number_of_provinces,
                                state.transport_timber + state.available_wagons);
                            break;
                        case 'w':
                        case 'W':
                            resource_name = "wool";
                            transport_order = &state.transport_wool;
                            max_transport = MIN(state.wool_yield_per_province * state.number_of_provinces,
                                state.transport_wool + state.available_wagons);
                            break;
                        case 'i':
                        case 'I':
                            resource_name = "iron";
                            transport_order = &state.transport_iron;
                            max_transport = MIN(state.iron_yield_per_province * state.number_of_provinces,
                                state.transport_iron + state.available_wagons);
                            break;
                        case 'c':
                        case 'C':
                            resource_name = "coal";
                            transport_order = &state.transport_coal;
                            max_transport = MIN(state.coal_yield_per_province * state.number_of_provinces,
                                state.transport_coal + state.available_wagons);
                            break;
                        default:
                            continue;
                    }

                    max_transport = MIN(max_transport, MAX_UCHAR);
                    while (1) {
                        old_order = *transport_order;
                        clear_input_area();
                        print(5, 20, "Transport how many units of");
                        print(33, 20, resource_name);
                        print(5, 21, "per turn (Max:    )?");
                        print_int_right_aligned(22, 21, max_transport);
                        new_transport_order = scan_uint(25, 21, 5);
                        if (new_transport_order > max_transport) {
                            continue;
                        }

                        *transport_order = new_transport_order;
                        delta = (signed int)*transport_order - (signed int)old_order;
                        state.available_wagons -= delta;
                        break;
                    }
                    break;
                }
                break;
            case 'q':
            case 'Q':
                state.current_screen = SCREEN_INDUSTRY;
                return;
        }
    }
}
