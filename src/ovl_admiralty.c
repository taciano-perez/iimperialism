#include "game.h"

#define BOX_X1 0
#define BOX_Y1 8
#define BOX_X2 39
#define BOX_Y2 13

void render_admiralty_screen(void) {
    unsigned int max_units;
    unsigned int units_to_build;
    unsigned char build_cost;
    char key;

    while (1) {
        clear_screen();

        print(0, 0, "Admiralty Headquarters");
        render_turn_funds_header();

        render_warehouse_box();

        box(BOX_X1, BOX_Y1+1, BOX_X1+17, BOX_Y2);
        print_inverted((BOX_X1), BOX_Y1, "Merchant Marine");
        print((BOX_X1+1), (BOX_Y1+1), "Traders:");
        print_int_right_aligned((BOX_X1+14), (BOX_Y1+1), state.traders);
        print((BOX_X1+1), (BOX_Y1+2), "Cargo/trader:");
        print_int_right_aligned((BOX_X1+14), (BOX_Y1+2), state.capacity_per_trader);
        print((BOX_X1+1), (BOX_Y1+4), "Capacity:");
        print_int_right_aligned((BOX_X1+14), (BOX_Y1+4), state.traders * state.capacity_per_trader);

        box(BOX_X1+19, BOX_Y1+1, BOX_X2, BOX_Y2);
        print_inverted((BOX_X1+19), BOX_Y1, "Navy");
        print((BOX_X1+20), (BOX_Y1+1), "Warships:");
        print_int_right_aligned((BOX_X1+36), (BOX_Y1+1), state.frigates);
        print((BOX_X1+20), (BOX_Y1+2), "Guns/ship:");
        print_int_right_aligned((BOX_X1+36), (BOX_Y1+2), state.guns_per_frigate);
        print((BOX_X1+20), (BOX_Y1+4), "Firepower:");
        print_int_right_aligned((BOX_X1+36), (BOX_Y1+4), state.frigates * state.guns_per_frigate);

        print(1, 14, "Available workers:");
        print_int_right_aligned(24, 14, state.available_workers);

        draw_picture_at(ADMIRAL_PORTRAIT, 0, 20);
        print(5, 20, "Build Trader, Warship,");
        print(5, 21, "or Quit?");

        key = cgetc_at(13, 21);

        switch (key) {
            case 't':
            case 'T':
                clear_input_area();
                build_cost = state.capacity_per_trader / CAPACITY_PER_TRADER_BASE;
                max_units = MIN(MIN(state.resources[RESOURCE_LUMBER] / build_cost,
                                    state.resources[RESOURCE_FABRIC] / build_cost),
                                state.available_workers);
                max_units = MIN(max_units, MAX_UCHAR - state.traders);

                while (1) {
                    clear_input_area();
                    print(5, 20, "Trader cost:   lumber,   fabric");
                    print_int_right_aligned(18, 20, build_cost);
                    print_int_right_aligned(28, 20, build_cost);
                    print(5, 21, "& 1 worker.");
                    print(5, 22, "Build how many (Max:    )?");
                    print_int_right_aligned(28, 22, max_units);
                    units_to_build = scan_uint(30, 22, 3);
                    if (units_to_build > max_units) {
                        continue;
                    }

                    state.resources[RESOURCE_LUMBER] -= units_to_build * build_cost;
                    state.resources[RESOURCE_FABRIC] -= units_to_build * build_cost;
                    state.available_workers -= units_to_build;
                    state.traders += units_to_build;
                    break;
                }
                break;

            case 'w':
            case 'W':
                clear_input_area();
                build_cost = state.guns_per_frigate / GUNS_PER_FRIGATE_BASE;
                max_units = MIN(MIN(MIN(state.resources[RESOURCE_LUMBER], state.resources[RESOURCE_FABRIC]),
                                    state.resources[RESOURCE_GUNS] / build_cost),
                                state.available_workers);
                max_units = MIN(max_units, MAX_UCHAR - state.frigates);

                while (1) {
                    clear_input_area();
                    print(5, 20, get_diplomacy_string(DSTR_WARSHIP_COST));
                    print(5, 21, get_diplomacy_string(DSTR_WARSHIP_GUNS_WORKER));
                    print_int_right_aligned(5, 21, build_cost);
                    print(5, 22, "Build how many (Max:    )?");
                    print_int_right_aligned(28, 22, max_units);

                    units_to_build = scan_uint(30, 22, 3);
                    if (units_to_build > max_units) {
                        continue;
                    }

                    state.resources[RESOURCE_LUMBER] -= units_to_build;
                    state.resources[RESOURCE_FABRIC] -= units_to_build;
                    state.resources[RESOURCE_GUNS] -= units_to_build * build_cost;
                    state.available_workers -= units_to_build;
                    state.frigates += units_to_build;
                    break;
                }
                break;

            case 'q':
            case 'Q':
                state.current_screen = SCREEN_MAIN;
                return;
        }
    }
}
