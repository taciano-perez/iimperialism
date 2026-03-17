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
static unsigned char selected_trade_nation;

void render_warehouse_box() {
    box(BOX1_X1, BOX1_Y1+1, BOX1_X2, BOX1_Y2);
    print ((BOX1_X1+1), BOX1_Y1, "Warehouse");

    print((BOX1_X1+1), (BOX1_Y1+1), "Timber:");
    print_int_right_aligned((BOX1_X1+11), (BOX1_Y1+1), state.resources[RESOURCE_TIMBER]);
    print((BOX1_X1+1), (BOX1_Y1+2), "Wool:");
    print_int_right_aligned((BOX1_X1+11), (BOX1_Y1+2), state.resources[RESOURCE_WOOL]);
    print((BOX1_X1+1), (BOX1_Y1+3), "Iron:");
    print_int_right_aligned((BOX1_X1+11), (BOX1_Y1+3), state.resources[RESOURCE_IRON]);
    print((BOX1_X1+1), (BOX1_Y1+4), "Coal:");
    print_int_right_aligned((BOX1_X1+11), (BOX1_Y1+4), state.resources[RESOURCE_COAL]);

    print((BOX1_X1+13), (BOX1_Y1+1), "Lumber:");
    print_int_right_aligned((BOX1_X1+23), (BOX1_Y1+1), state.resources[RESOURCE_LUMBER]);
    print((BOX1_X1+13), (BOX1_Y1+2), "Fabric:");
    print_int_right_aligned((BOX1_X1+23), (BOX1_Y1+2), state.resources[RESOURCE_FABRIC]);
    print((BOX1_X1+13), (BOX1_Y1+3), "Steel:");
    print_int_right_aligned((BOX1_X1+23), (BOX1_Y1+3), state.resources[RESOURCE_STEEL]);

    print((BOX1_X1+25), (BOX1_Y1+1), "Furniture:");
    print_int_right_aligned((BOX1_X1+38), (BOX1_Y1+1), state.resources[RESOURCE_FURNITURE]);
    print((BOX1_X1+25), (BOX1_Y1+2), "Clothes:");
    print_int_right_aligned((BOX1_X1+38), (BOX1_Y1+2), state.resources[RESOURCE_CLOTHES]);
    print((BOX1_X1+25), (BOX1_Y1+3), "Tools:");
    print_int_right_aligned((BOX1_X1+38), (BOX1_Y1+3), state.resources[RESOURCE_TOOLS]);
    print((BOX1_X1+25), (BOX1_Y1+4), "Guns:");
    print_int_right_aligned((BOX1_X1+38), (BOX1_Y1+4), state.resources[RESOURCE_GUNS]);
}

void set_selected_trade_nation(unsigned char nation_index) {
    selected_trade_nation = nation_index;
}

unsigned char get_selected_trade_nation(void) {
    return selected_trade_nation;
}

int main(void) {

    char key;
    init_overlays();
    ui_init();
    seed_random(1);
    init_game();
    while (1) { // main game loop
        if (state.current_screen == SCREEN_INDUSTRY) {
            run_overlay(OVL_INDUSTRY);
        } else if (state.current_screen == SCREEN_TRANSPORT) {
            run_overlay(OVL_TRANSPORT);
        } else if (state.current_screen == SCREEN_PRODUCTION) {
            run_overlay(OVL_PRODUCTION);
        } else if (state.current_screen == SCREEN_ADMIRALTY) {
            run_overlay(OVL_ADMIRALTY);
            continue;
        } else if (state.current_screen == SCREEN_DIPLOMACY) {
            run_overlay(OVL_DIPLOMACY);
            continue; // skip input handling and go directly to next screen
        } else if (state.current_screen == SCREEN_TRADE_EXPEDITION) {
            run_overlay(OVL_TRADE_EXPEDITION);
            run_overlay(OVL_TRADE_EXPEDITION_ACTION);
            continue;
        } else if (state.current_screen == SCREEN_BATTLE) {
            clear_screen();
            draw_picture_at(ADMIRAL_PORTRAIT, 0, 20);
            print(5, 20, "Ambushed by pirates!");
            run_overlay(OVL_BATTLE);
        } else if (state.current_screen == SCREEN_SCIENCE) {
            run_overlay(OVL_SCIENCE);
            continue;
        }

        key = cgetc();

        switch (key) {
            case 27: // ESC key
                run_overlay(OVL_GAME_MENU);
                continue;
            default:
                if (state.current_screen == SCREEN_INDUSTRY) {
                    handle_screen_input_industry(key);
                } else  if (state.current_screen == SCREEN_TRANSPORT) {
                    handle_screen_input_transport(key);
                } else if (state.current_screen == SCREEN_PRODUCTION) {
                    handle_screen_input_production(key);
                }
                break;
        }
    }

}
