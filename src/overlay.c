#include <stdio.h>
#include "game.h"
#include "overlay.h"

/* Assembly routines from overlay.s */
extern void install_trampoline(void);
extern void main_to_aux(unsigned int src, unsigned int dst, unsigned char pages);

#pragma code-name (push, "LOWCODE")

/* Load one overlay binary from a ProDOS file into AUX RAM.
 * The file is read into OVERLAY_SLOT (MAIN $8800) and then
 * copied page-by-page from MAIN to AUX via RAMWRT. */
static void load_overlay_file(const char* filename, unsigned int aux_dst) {
    FILE* f;
    f = fopen(filename, "rb");
    if (f) {
        fread((void*)OVERLAY_SLOT, 1, OVERLAY_SIZE, f);
        fclose(f);
    }
    main_to_aux(OVERLAY_SLOT, aux_dst, OVERLAY_PAGES);
}

/* Load all three overlay binaries from disk into AUX RAM, then install the
 * RAMRD trampoline in the hardware stack page ($0100-$0117).
 *
 * AUX layout after init:
 *   $8800-$8FFF  OVL_INDUSTRY   (OVERLAY_PAGES pages)
 *   $9000-$97FF  OVL_PRODUCTION (OVERLAY_PAGES pages)
 *   $9800-$9FFF  OVL_TRANSPORT  (OVERLAY_PAGES pages)
 *
 * OVERLAY_SLOT ($8800 in MAIN) is used as a temporary load buffer
 * and is freely overwritten by run_overlay() later. */
void init_overlays(void) {
    install_trampoline();
    load_overlay_file(OVL_FILE_INDUSTRY,   0x8800u);
    load_overlay_file(OVL_FILE_PRODUCTION, 0x9000u);
    load_overlay_file(OVL_FILE_TRANSPORT,  0x9800u);
    load_overlay_file(OVL_FILE_ADMIRALTY,  0xA000u);}

/* Copy overlay <id> from AUX RAM into OVERLAY_SLOT ($8800 in MAIN) and
 * execute it, passing &state as the GameState* argument. */
void run_overlay(unsigned char id) {
    unsigned int aux_addr;

    /* AUX base for each overlay: $8800 + id * $0800 */
    aux_addr = (unsigned int)0x8800u +
               (unsigned int)id * (unsigned int)OVERLAY_SIZE;

    /* Set trampoline ZP parameters for the AUX->MAIN copy */
    tramp_src[0]  = (unsigned char)aux_addr;
    tramp_src[1]  = (unsigned char)(aux_addr >> 8);
    tramp_dst[0]  = (unsigned char)OVERLAY_SLOT;
    tramp_dst[1]  = (unsigned char)(OVERLAY_SLOT >> 8);
    tramp_pages   = OVERLAY_PAGES;

    /* Call trampoline at $0100: copies OVERLAY_PAGES from AUX to MAIN $8800 */
    ((void(*)(void))0x0100u)();

    /* Execute the overlay, passing a pointer to the game state */
    ((void(*)(GameState*))OVERLAY_SLOT)(&state);
}

#pragma code-name (pop)
