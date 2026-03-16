#include "game.h"

#define BOX_X1 0
#define BOX_Y1 8
#define BOX_X2 39
#define BOX_Y2 13

void render_admiralty_screen(void) {
    clear_screen();

    print(0,  0, "Admiralty Headquarters");
    print(34, 0, "Turn:");
    print_int_right_aligned(39, 0, state.turn_number);

    render_warehouse_box();

    box(BOX_X1, BOX_Y1+1, BOX_X1+17, BOX_Y2);
    print((BOX_X1+1), BOX_Y1, "Merchant Fleet");
    print((BOX_X1+1), (BOX_Y1+1), "Traders:");
    print_int_right_aligned((BOX_X1+14), (BOX_Y1+1), state.traders);
    print((BOX_X1+1), (BOX_Y1+2), "Cargo/trader:");
    print_int_right_aligned((BOX_X1+14), (BOX_Y1+2), state.capacity_per_trader);
    print((BOX_X1+1), (BOX_Y1+4), "Capacity:");
    print_int_right_aligned((BOX_X1+14), (BOX_Y1+4), state.traders * state.capacity_per_trader);

    box(BOX_X1+19, BOX_Y1+1, BOX_X2, BOX_Y2);
    print((BOX_X1+20), BOX_Y1, "Navy");
    print((BOX_X1+20), (BOX_Y1+1), "Warships:");
    print_int_right_aligned((BOX_X1+36), (BOX_Y1+1), state.frigates);
    print((BOX_X1+20), (BOX_Y1+2), "Guns/ship:");
    print_int_right_aligned((BOX_X1+36), (BOX_Y1+2), state.guns_per_frigate);
    print((BOX_X1+20), (BOX_Y1+4), "Firepower:");
    print_int_right_aligned((BOX_X1+36), (BOX_Y1+4), state.frigates * state.guns_per_frigate);

    print(1, 14, "Available workers:");
    print_int_right_aligned(24, 14, state.available_workers);

    /* ADVISOR */
    draw_picture_at(ADMIRAL_PORTRAIT, 0, 20);
    print(5, 20, "Awaiting orders, sir.");
    print(5, 21, "Build Trading vessel, Warship,");
    print(5, 22, "or Quit?");
}
