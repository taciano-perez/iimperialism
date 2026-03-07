#include <stdio.h>
#include <string.h>
#include "game.h"

#define SAVE_FILE "GAME.DATA"
#define MAGIC_NUMBER 0x4947  /* "IG" for Imperialism Game */
#define VERSION 2

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
    FILE* fp;
    unsigned int magic = MAGIC_NUMBER;
    unsigned char version = VERSION;

    fp = fopen(SAVE_FILE, "wb");
    if (fp == NULL) {
        return -1;
    }

    /* Write magic number and version for file validation */
    if (fwrite(&magic, sizeof(magic), 1, fp) != 1) {
        fclose(fp);
        return -1;
    }

    if (fwrite(&version, sizeof(version), 1, fp) != 1) {
        fclose(fp);
        return -1;
    }

    /* Write the entire game state structure */
    if (fwrite(state, sizeof(GameState), 1, fp) != 1) {
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
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
    FILE* fp;
    unsigned int magic;
    unsigned char version;

    fp = fopen(SAVE_FILE, "rb");
    if (fp == NULL) {
        return -1;
    }

    /* Read and validate magic number */
    if (fread(&magic, sizeof(magic), 1, fp) != 1) {
        fclose(fp);
        return -1;
    }

    if (magic != MAGIC_NUMBER) {
        fclose(fp);
        return -1;
    }

    /* Read and validate version */
    if (fread(&version, sizeof(version), 1, fp) != 1) {
        fclose(fp);
        return -1;
    }

    if (version != VERSION) {
        fclose(fp);
        return -1;
    }

    /* Read the entire game state structure */
    if (fread(state, sizeof(GameState), 1, fp) != 1) {
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

#pragma code-name (pop)
