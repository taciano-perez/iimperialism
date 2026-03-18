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
#define TRADE_EXPEDITION_BATTLE_CHANCE_PERCENT 100U
#define BATTLE_TRADER_HIT_CHANCE_PERCENT 100U
#define SCIENCE_RESEARCH_COST_MULTIPLIER 1000U
#define FOREIGN_NATION_COUNT 5
#define FOREIGN_TRADE_ENTRY_COUNT 3
#define FOREIGN_NATION_NAME_LENGTH 6
#define SCIENCE_LEVEL_COUNT 7

typedef struct {
    char name[FOREIGN_NATION_NAME_LENGTH + 1];
    unsigned char relations;
    unsigned char exports[FOREIGN_TRADE_ENTRY_COUNT];
    unsigned char imports[FOREIGN_TRADE_ENTRY_COUNT];
    unsigned char export_prices[FOREIGN_TRADE_ENTRY_COUNT];
    unsigned char import_prices[FOREIGN_TRADE_ENTRY_COUNT];
} ForeignNation;

/* ============================================================================
 * Game State Structure
 * ============================================================================
 */
typedef struct {
    /* Resource inventory, indexed by RESOURCE_* constants. */
    unsigned int resources[RESOURCE_COUNT];

    /* Provincial yields */
    unsigned char number_of_provinces;
    unsigned char timber_yield_per_province;
    unsigned char wool_yield_per_province;
    unsigned char iron_yield_per_province;
    unsigned char coal_yield_per_province;

    /* Transport */
    unsigned char transport_timber;
    unsigned char transport_wool;
    unsigned char transport_iron;
    unsigned char transport_coal;
    unsigned char available_wagons;

    /* Production orders */
    unsigned char production_lumber;
    unsigned char production_fabric;
    unsigned char production_steel;
    unsigned char production_furniture;
    unsigned char production_clothes;
    unsigned char production_tools;
    unsigned char production_guns;
    unsigned int available_workers;

    /* Navy */
    unsigned char traders;
    unsigned char frigates;
    /* Cached science-adjusted navy stats used by overlays and turn logic. */
    unsigned char capacity_per_trader;
    unsigned char guns_per_frigate;

    /* Foreign nations */
    ForeignNation foreign_nations[FOREIGN_NATION_COUNT];

    /* Game metadata */
    unsigned int turn_number;
    unsigned long money;
    /* Highest patented science level, from 0 up to the current tree limit. */
    unsigned char science_level;
    char nation_name[20];
    unsigned char current_screen;

    /* turn-specific data */
    unsigned char remaining_turn_capacity;
} GameState;

extern GameState state;

// logic.c
void init_game();
void next_turn();
void seed_random(unsigned int seed);
unsigned char rand_range(unsigned char min, unsigned char max);

// ui.c
void ui_init();
void ui_exit();
void clear_screen();
void clear_input_area();
void clear_area(unsigned char x, unsigned char y, unsigned char width, unsigned char height);
void paint_area(unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned char color);
void print (unsigned char x, unsigned char y, const char* text);
void print_right_aligned(unsigned char x, unsigned char y, const char* text);
void print_int_right_aligned(unsigned char x, unsigned char y, unsigned int value);
void print_int(unsigned char x, unsigned char y, unsigned int value);
void draw_picture_at(const unsigned char picture_index, const unsigned char x_byte, unsigned char y);
void box (unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2);
char cgetc_at(unsigned char x, unsigned char y);
unsigned int scan_uint(unsigned char x, unsigned char y, unsigned char max_digits);

// main.c (resident — accessible to overlays via jump table at $0821)
void render_warehouse_box(void);
const char* get_resource_name(unsigned char resource);
const char* get_relation_name(unsigned char relation);
void set_selected_trade_nation(unsigned char nation_index);
unsigned char get_selected_trade_nation(void);
unsigned char rand_range(unsigned char min, unsigned char max);

#endif // GAME_H
