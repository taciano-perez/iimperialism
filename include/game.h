#ifndef GAME_H
#define GAME_H

#define MIN(x, y) ((x) < (y) ? (x) : (y))

/* ============================================================================
 * Picture Index Constants
 * ============================================================================
 * Use these constants to reference pictures by name
 */
#define WISEMAN_PORTRAIT 0
#define INDUSTRY_PORTRAIT 1
#define ADMIRAL_PORTRAIT 2
/* Add more picture indices here as you add pictures */

/* ============================================================================
 * Screen Index Constants
 * ============================================================================
 */
#define SCREEN_MAIN 0
#define SCREEN_INDUSTRY 1
#define SCREEN_TRANSPORT 2
#define SCREEN_PRODUCTION 3
#define SCREEN_ADMIRALTY 4

/* ============================================================================
 * Gameplay constants
 * ============================================================================
 */
#define CAPACITY_PER_TRADER 3
#define GUNS_PER_FRIGATE 2

/* ============================================================================
 * Game State Structure
 * ============================================================================
 */
typedef struct {
    /* Raw materials */
    unsigned int timber;
    unsigned int wool;
    unsigned int iron;
    unsigned int coal;

    /* Processed materials */
    unsigned int lumber;
    unsigned int fabric;
    unsigned int steel;

    /* Finished goods */
    unsigned int furniture;
    unsigned int clothes;
    unsigned int tools;
    unsigned int guns;

    /* Provincial yields */
    unsigned int number_of_provinces;
    unsigned int timber_yield_per_province;
    unsigned int wool_yield_per_province;
    unsigned int iron_yield_per_province;
    unsigned int coal_yield_per_province;

    /* Transport */
    unsigned int transport_timber;
    unsigned int transport_wool;
    unsigned int transport_iron;
    unsigned int transport_coal;
    unsigned int available_wagons;

    /* Production orders */
    unsigned int production_lumber;
    unsigned int production_fabric;
    unsigned int production_steel;
    unsigned int production_furniture;
    unsigned int production_clothes;
    unsigned int production_tools;
    unsigned int production_guns;
    unsigned int available_workers;

    /* Navy */
    unsigned int traders;
    unsigned int frigates;

    /* Game metadata */
    unsigned int turn_number;
    char nation_name[20];
    unsigned int current_screen;
} GameState;

extern GameState state;

// logic.c
void init_game();
void next_turn();

// ui.c
void ui_init();
void ui_exit();
void clear_screen();
void clear_input_area();
void print (const int x, const int y, const char* text);
void print_right_aligned(const int x, const int y, const char* text);
void print_int_right_aligned(int x, int y, unsigned int value);
void print_int(const int x, const int y, unsigned int value);
void draw_picture_at(const unsigned char picture_index, const unsigned char x_byte, unsigned char y);
void box (const int x1, const int y1, const int x2, const int y2);
char cgetc_at(int x, int y);
unsigned int scan_uint(int x, int y, unsigned int max_digits);

// main.c (resident — accessible to overlays via jump table at $0821)
void render_warehouse_box(void);

// gamestate.c
int save_game(const GameState* state);
int load_game(GameState* state);

#endif // GAME_H
