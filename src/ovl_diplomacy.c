#include "game.h"

#define BOX_X1 0
#define BOX_Y1 2
#define BOX_X2 39
#define BOX_Y2 19

#define state (*s)

const char STR_STEEL[] = "Steel";
const char STR_FURNITURE[] = "Furniture";
const char STR_GUNS[] = "Guns";
const char STR_WOOL[] = "Wool";
const char STR_TIMBER[] = "Timber";
const char STR_COAL[] = "Coal";

void render_diplomacy_screen(GameState *s) {
    clear_screen();
    print(0, 0, "Diplomatic Office");
    print(34, 0, "Turn:");
    print_int_right_aligned(39, 0, state.turn_number);

    // box(BOX_X1, BOX_Y1+1, BOX_X2, BOX_Y2);
    print(1, 2, "Nation");
    print(10, 2, "Relations");
    print(20, 2, "Exports");
    print(30, 2, "Imports");
    
    // ORDUNE
    box(BOX_X1, BOX_Y1+1, BOX_X2, BOX_Y1+4);
    print(1, 4, "1)Ordune");
    print(10, 4, "Neutral");
    print(20, 3, STR_STEEL);
    print(20, 4, STR_FURNITURE);
    print(20, 5, STR_GUNS);
    print(30, 3, STR_WOOL);
    print(30, 4, STR_TIMBER);
    print(30, 5, STR_COAL);

    // DENEB
    box(BOX_X1, BOX_Y1+4, BOX_X2, BOX_Y1+7);
    print(1, 7, "2)Deneb");
    print(10, 7, "Good");
    print(20, 6, STR_STEEL);
    print(20, 7, STR_FURNITURE);
    print(20, 8, STR_GUNS);
    print(30, 6, STR_WOOL);
    print(30, 7, STR_TIMBER);
    print(30, 8, STR_COAL);

    // LOKE
    box(BOX_X1, BOX_Y1+7, BOX_X2, BOX_Y1+10);
    print(1, 10, "3)Loke");
    print(10, 10, "Excellent");
    print(20, 9, STR_STEEL);
    print(20, 10, STR_FURNITURE);
    print(20, 11, STR_GUNS);
    print(30, 9, STR_WOOL);
    print(30, 10, STR_TIMBER);
    print(30, 11, STR_COAL);

    // PONT
    box(BOX_X1, BOX_Y1+10, BOX_X2, BOX_Y1+13);
    print(1, 13, "4)Pont");
    print(10, 13, "Bad");
    print(20, 12, STR_STEEL);
    print(20, 13, STR_FURNITURE);
    print(20, 14, STR_GUNS);
    print(30, 12, STR_WOOL);
    print(30, 13, STR_TIMBER);
    print(30, 14, STR_COAL);

    // KATHAY
    box(BOX_X1, BOX_Y1+13, BOX_X2, BOX_Y1+16);
    print(1, 16, "5)Kathay");
    print(10, 16, "Good");
    print(20, 15, STR_STEEL);
    print(20, 16, STR_FURNITURE);
    print(20, 17, STR_GUNS);
    print(30, 15, STR_WOOL);
    print(30, 16, STR_TIMBER);
    print(30, 17, STR_COAL);

    draw_picture_at(WISEMAN_PORTRAIT, 0, 20);
    print(5, 20, "Your diplomats await orders.");
    // print(5, 21, "");
    // print(5, 22, "");
}
