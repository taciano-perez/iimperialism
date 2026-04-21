#ifndef GAME_H
#define GAME_H

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MAX_UINT 65535U
#define MAX_ULONG 4294967295UL
#define MAX_UCHAR 255U
#define TRUE 1U
#define FALSE 0U

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
#define SHIP_FOREIGN 6
#define SHIP_SPLASH 7

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
#define SCREEN_COUNCIL_NATIONS 9
#define SCREEN_LEDGER 10

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
#define RELATION_ALLY_COLONY 255

#define RELTYPE_COLONY 0U
#define RELTYPE_ALLIANCE 1U

#define INDEX_PIRATES 255U

/* ============================================================================
 * Gameplay constants
 * ============================================================================
 */
#define CAPACITY_PER_TRADER_BASE 2
#define GUNS_PER_FRIGATE_BASE 2
#define TRADE_EXPEDITION_BATTLE_CHANCE_PERCENT 50U
#define TRADE_EXPEDITION_ATTACK_FOREIGN_NATION_CHANCE_PERCENT 50U
#define BATTLE_TRADER_HIT_CHANCE_PERCENT 5U
#define BATTLE_BOUNTY_VARIANCE_PERCENT 50U
#define BATTLE_RUN_TRADER_HIT_CHANCE_PERCENT 60U
#define DIPLOMATIC_OVERTURE_CHANCE_PERCENT 50U
#define DIPLOMATIC_OFFER_COST 1000U
#define TRADE_RELATIONS_MULTIPLIER 2U
#define FOREIGN_EXPORT_PRICE_BASE_PERCENT 85U
#define FOREIGN_EXPORT_PRICE_VARIANCE_PERCENT 15U
#define FOREIGN_EXPORT_PRICE_RELATION_STEP_PERCENT 3U
#define FOREIGN_IMPORT_PRICE_BASE_PERCENT 110U
#define FOREIGN_IMPORT_PRICE_VARIANCE_PERCENT 20U
#define FOREIGN_IMPORT_PRICE_RELATION_STEP_PERCENT 4U
#define SCIENCE_RESEARCH_COST_MULTIPLIER 1000U
#define RELATIONS_LOSS_PER_TURN 3U
#define FOREIGN_NATION_COUNT 5
#define FOREIGN_TRADE_ENTRY_COUNT 3
#define FOREIGN_NATION_NAME_LENGTH 6
#define SCIENCE_LEVEL_COUNT 9
#define UPKEEP_COST_PER_WORKER 2U
#define UPKEEP_COST_PER_TRADER 5U
#define UPKEEP_COST_PER_WARSHIP 5U

#define DSTR_BUY_SELL_QUIT 21U
#define DSTR_COMMODITY_BUY 22U
#define DSTR_COMMODITY_SELL 23U
#define DSTR_HOW_MANY 24U
#define DSTR_WARSHIP_COST 25U
#define DSTR_WARSHIP_GUNS_WORKER 26U

/* ============================================================================
 * Council of Nations constants
 * ============================================================================
 */
#define COUNCIL_NATION_COUNT 6U
#define COUNCIL_GREAT_POWER_COUNT 3U
#define COUNCIL_MAJOR_POWER_PROVINCES 8U
#define COUNCIL_MINOR_POWER_PROVINCES 4U
#define COUNCIL_VICTORY_VOTES 24U
#define COUNCIL_TOTAL_PROVINCES 32U

/* ============================================================================
 * Victory score constants
 * ============================================================================
 */
#define SCORE_WEIGHT_DIPLOMACY 50U
#define SCORE_WEIGHT_TREASURY 25U
#define SCORE_WEIGHT_SPEED 25U

#define SCORE_SCALE_FACTOR 560U

#define SCORE_TREASURY_1 10000UL
#define SCORE_TREASURY_2 25000UL
#define SCORE_TREASURY_3 50000UL
#define SCORE_TREASURY_4 75000UL
#define SCORE_TREASURY_5 100000UL

#define SCORE_SPEED_TURN_1 10U
#define SCORE_SPEED_TURN_2 20U
#define SCORE_SPEED_TURN_3 30U
#define SCORE_SPEED_TURN_4 40U
#define SCORE_SPEED_TURN_5 50U

#define SCORE_SPEED_SCORE_1 100U
#define SCORE_SPEED_SCORE_2 90U
#define SCORE_SPEED_SCORE_3 75U
#define SCORE_SPEED_SCORE_4 60U
#define SCORE_SPEED_SCORE_5 45U
#define SCORE_SPEED_SCORE_6 30U

#define SCORE_RANK_VICTORIA 0U
#define SCORE_RANK_BISMARCK 1U
#define SCORE_RANK_NAPOLEON 2U
#define SCORE_RANK_CHARLES 3U
#define SCORE_RANK_FERDINAND 4U
#define SCORE_RANK_COUNT 5U

#define SCORE_RANK_THRESHOLD_VICTORIA 50000U
#define SCORE_RANK_THRESHOLD_BISMARCK 35000U
#define SCORE_RANK_THRESHOLD_NAPOLEON 20000U
#define SCORE_RANK_THRESHOLD_CHARLES 8000U

typedef struct {
    char name[FOREIGN_NATION_NAME_LENGTH + 1];
    unsigned char relations;
    unsigned char relations_previous_turn;
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
    unsigned int trade_expenses;
    unsigned int trade_revenue;
    unsigned int turn_booty;
    unsigned char attacker_index; // INDEX_PIRATES or index of foreign nation attacking in battle screen
} GameState;

extern GameState state;

// logic.c
void init_game();
void next_turn();
void copy_text_limited(char* dest, const char* src, unsigned char capacity);
unsigned char get_relation_tier(unsigned char relations);
void seed_random(unsigned int seed);
unsigned char rand_range(unsigned char min, unsigned char max);
void production_orders(void);
void build_final_score_line(char* buffer);
unsigned char get_final_rank_index(void);
const char* get_final_victory_string(unsigned char index);

// ui.c
void ui_init();
void ui_exit();
void clear_screen();
void clear_input_area();
void clear_area(unsigned char x, unsigned char y, unsigned char width, unsigned char height);
void paint_area(unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned char color);
void print (unsigned char x, unsigned char y, const char* text);
void print_inverted(unsigned char x, unsigned char y, const char* text);
void print_bold(unsigned char x, unsigned char y, const char* text);
void print_right_aligned(unsigned char x, unsigned char y, const char* text);
void format_uint(char* buffer, unsigned int value);
void format_money(char* buffer, unsigned long value);
void print_int_right_aligned(unsigned char x, unsigned char y, unsigned int value);
void print_int_right_aligned_currency(unsigned char x, unsigned char y, unsigned long value);
void print_signed_int_right_aligned_currency(unsigned char x, unsigned char y, int value);
void print_int(unsigned char x, unsigned char y, unsigned int value);
void draw_picture_at(const unsigned char picture_index, const unsigned char x_byte, unsigned char y);
void draw_picture_offset_at(const unsigned char picture_index, const unsigned char x_byte, const unsigned char x_offset, unsigned char y);
void box (unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2);
void wait_three_seconds_or_keypress(void);
char cgetc_at(unsigned char x, unsigned char y);
unsigned int scan_uint(unsigned char x, unsigned char y, unsigned char max_digits);
void scan_text(unsigned char x, unsigned char y, char* buffer, unsigned char max_length);
void play_sound_alert();

// main.c (resident — accessible to overlays via jump table at $0821)
void render_warehouse_box(void);
void render_turn_funds_header(void);
void render_trade_market(void);
void start_new_game(void);
const char* get_resource_name(unsigned char resource);
const char* get_diplomacy_string(unsigned char index);
const char* get_relation_name(unsigned char relation, unsigned char nation_index);
void set_selected_trade_nation(unsigned char nation_index);
unsigned char get_selected_trade_nation(void);
unsigned char rand_range(unsigned char min, unsigned char max);

#endif // GAME_H
