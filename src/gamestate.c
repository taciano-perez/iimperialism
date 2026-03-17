#include "game.h"

/* Direct ProDOS MLI helpers implemented in asm/prodos_gamestate_io.s. They
 * return 0 on success and a non-zero ProDOS/local validation code on failure. */
extern unsigned char __fastcall__ prodos_save_game(const GameState* state);
extern unsigned char __fastcall__ prodos_load_game(GameState* state);

/* Exported for the assembly helper so it can read/write the exact payload size
 * without duplicating GameState layout knowledge. */
const unsigned int game_state_size = sizeof(GameState);

#pragma code-name (push, "LOWCODE")

/* ============================================================================
 * save_game
 * ============================================================================
 * Saves the game state to GAME.DATA file.
 *
 * Parameters:
 *   state - Pointer to the GameState structure to save
 *
 * Returns:
 *   0 on success, -1 on failure
 */
int save_game(const GameState* state) {
    /* Preserve the existing C API: 0 success, -1 failure. */
    return prodos_save_game(state) == 0 ? 0 : -1;
}

/* ============================================================================
 * load_game
 * ============================================================================
 * Loads the game state from GAME.DATA file.
 *
 * Parameters:
 *   state - Pointer to the GameState structure to load into
 *
 * Returns:
 *   0 on success, -1 on failure
 */
int load_game(GameState* state) {
    /* Preserve the existing C API: 0 success, -1 failure. */
    return prodos_load_game(state) == 0 ? 0 : -1;
}

#pragma code-name (pop)
