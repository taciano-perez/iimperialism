#include <conio.h>
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "overlay.h"
#include "screens.h"

#define BOX1_X1 0
#define BOX1_Y1 2
#define BOX1_X2 39
#define BOX1_Y2 7

// Global state instance
GameState state;

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
    print((BOX1_X1+25), (BOX1_Y1+4), "Guns: ");
    print_int_right_aligned((BOX1_X1+39), (BOX1_Y1+4), state.guns);
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
