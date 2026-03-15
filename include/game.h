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
#define SCIENCE_PORTRAIT 3
#define SHIP_PIRATE 4
#define SHIP 5

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
#define SCREEN_DIPLOMACY 5
#define SCREEN_TRADE_EXPEDITION 6
#define SCREEN_BATTLE 7
#define SCREEN_SCIENCE 8

/* ============================================================================
 * Resource Constants
 * ============================================================================
 */
#define RESOURCE_TIMBER 0
#define RESOURCE_WOOL 1 
#define RESOURCE_IRON 2
#define RESOURCE_COAL 3
#define RESOURCE_LUMBER 4
#define RESOURCE_FABRIC 5
#define RESOURCE_STEEL 6
#define RESOURCE_FURNITURE 7
#define RESOURCE_CLOTHES 8
#define RESOURCE_TOOLS 9
#define RESOURCE_GUNS 10
#define RESOURCE_COUNT 11

/* ============================================================================
 * Diplomacy Constants
 * ============================================================================
 */
#define RELATION_TERRIBLE 0
#define RELATION_BAD 50
#define RELATION_NEUTRAL 100  
#define RELATION_GOOD 150
#define RELATION_EXCELLENT 200

/* ============================================================================
 * Gameplay constants
 * ============================================================================
 */
#define CAPACITY_PER_TRADER_BASE 2
#define GUNS_PER_FRIGATE_BASE 2
#define SCIENCE_RESEARCH_COST_MULTIPLIER 1000U
#define FOREIGN_NATION_COUNT 5
#define FOREIGN_TRADE_ENTRY_COUNT 3
#define FOREIGN_NATION_NAME_LENGTH 6

typedef struct {
    char name[FOREIGN_NATION_NAME_LENGTH + 1];
    unsigned int relations;
    unsigned int exports[FOREIGN_TRADE_ENTRY_COUNT];
    unsigned int imports[FOREIGN_TRADE_ENTRY_COUNT];
    unsigned int export_prices[FOREIGN_TRADE_ENTRY_COUNT];
    unsigned int import_prices[FOREIGN_TRADE_ENTRY_COUNT];
} ForeignNation;

/* ============================================================================
 * Game State Structure
 * ============================================================================
 */
typedef struct {
    /* Resource inventory, indexed by RESOURCE_* constants. */
    unsigned int resources[RESOURCE_COUNT];

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
    /* Cached science-adjusted navy stats used by overlays and turn logic. */
    unsigned char capacity_per_trader;
    unsigned char guns_per_frigate;

    /* Foreign nations */
    ForeignNation foreign_nations[FOREIGN_NATION_COUNT];

    /* Game metadata */
    unsigned int turn_number;
    unsigned int money;
    /* Highest patented science level, from 0 up to the current tree limit. */
    unsigned char science_level;
    char nation_name[20];
    unsigned int current_screen;

    /* turn-specific data */
    unsigned int remaining_turn_capacity;
} GameState;

extern GameState state;

// logic.c
void init_game();
void next_turn();
void seed_random(unsigned int seed);
unsigned int rand_range(unsigned int min, unsigned int max);

// ui.c
void ui_init();
void ui_exit();
void clear_screen();
void clear_input_area();
void clear_area(int x, int y, int width, int height);
void paint_area(int x, int y, int width, int height, unsigned char color);
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
const char* get_resource_name(unsigned int resource);
const char* get_relation_name(unsigned int relation);
void set_selected_trade_nation(unsigned char nation_index);
unsigned char get_selected_trade_nation(void);
unsigned int get_trade_max_quantity(unsigned char mode, unsigned int resource, unsigned int price);
void apply_trade(unsigned char mode, unsigned int resource, unsigned int quantity, unsigned int price);
unsigned int rand_range(unsigned int min, unsigned int max);

// gamestate.c
int save_game(const GameState* state);
int load_game(GameState* state);

#endif // GAME_H
