#include "game.h"

#define BOX_X1 0
#define BOX_Y1 8
#define BOX_X2 39
#define BOX_Y2 13

void render_transport_screen(void) {
    clear_screen();
    print(0, 0, "Transport Orders");
    print(34, 0, "Turn:");
    print_int_right_aligned(39, 0, state.turn_number);
    render_warehouse_box();

    box(BOX_X1, BOX_Y1, BOX_X2, BOX_Y2);
    print((BOX_X1+1), BOX_Y1, "Transport Orders");
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
    print(5, 22, "Build wagons or Quit?");
}
