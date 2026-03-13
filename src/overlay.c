#include "game.h"
#include "overlay.h"

/* Assembly routines from overlay.s */
extern void install_trampoline(void);
extern void main_to_aux(unsigned int src, unsigned int dst, unsigned char pages);
extern unsigned char __fastcall__ prodos_load_overlay(const char* filename);
extern unsigned int overlay_bytes_read;

#pragma code-name (push, "LOWCODE")

static const char* overlay_filename(unsigned char id) {
    switch (id) {
        case OVL_INDUSTRY:   return OVL_FILE_INDUSTRY;
        case OVL_PRODUCTION: return OVL_FILE_PRODUCTION;
        case OVL_TRANSPORT:  return OVL_FILE_TRANSPORT;
        case OVL_ADMIRALTY:  return OVL_FILE_ADMIRALTY;
        case OVL_ADMIRALTY_TRADER:  return OVL_FILE_ADMIRALTY_TRADER;
        case OVL_ADMIRALTY_WARSHIP: return OVL_FILE_ADMIRALTY_WARSHIP;
        case OVL_DIPLOMACY: return OVL_FILE_DIPLOMACY;
        case OVL_TRADE_EXPEDITION: return OVL_FILE_TRADE_EXPEDITION;
        case OVL_TRADE_EXPEDITION_ACTION: return OVL_FILE_TRADE_EXPEDITION_ACTION;
        case OVL_BATTLE: return OVL_FILE_BATTLE;
        default:             return 0;
    }
}

static void overlay_load_failed(const char* filename, const char* reason, unsigned int detail) {
    clear_screen();
    print(1, 1, "Overlay load failed");
    if (filename) {
        print(1, 3, filename);
    } else {
        print(1, 3, "Invalid overlay id");
    }
    if (reason) {
        print(1, 5, reason);
        print_int(1, 6, detail);
    }
    while (1) {
    }
}

/* Load one overlay binary from disk into OVERLAY_SLOT (MAIN $8800). */
static void load_overlay_file(const char* filename) {
    unsigned char error_code;

    error_code = prodos_load_overlay(filename);
    if (error_code != 0) {
        overlay_load_failed(filename, "ProDOS err:", (unsigned int)error_code);
    }

    if (overlay_bytes_read != OVERLAY_SIZE) {
        overlay_load_failed(filename, "read bytes:", overlay_bytes_read);
    }
}

/* Install the RAMRD trampoline in the hardware stack page ($0100-$0117).
 * Overlay binaries are now loaded on demand by run_overlay(). */
void init_overlays(void) {
    install_trampoline();
}

/* Load overlay <id> into OVERLAY_SLOT ($8800 in MAIN), mirror it to AUX
 * $8800-$8FFF, copy it back into MAIN via the trampoline, and execute it. */
void run_overlay(unsigned char id) {
    const char* filename;

    filename = overlay_filename(id);
    if (!filename) {
        overlay_load_failed(0, 0, 0);
    }

    load_overlay_file(filename);

    /* Keep a single cached copy in AUX RAM at the same address. */
    main_to_aux(OVERLAY_SLOT, OVERLAY_SLOT, OVERLAY_PAGES);

    /* Set trampoline ZP parameters for the AUX->MAIN copy. */
    tramp_src[0]  = (unsigned char)OVERLAY_SLOT;
    tramp_src[1]  = (unsigned char)(OVERLAY_SLOT >> 8);
    tramp_dst[0]  = (unsigned char)OVERLAY_SLOT;
    tramp_dst[1]  = (unsigned char)(OVERLAY_SLOT >> 8);
    tramp_pages   = OVERLAY_PAGES;

    /* Call trampoline at $0100: copies OVERLAY_PAGES from AUX to MAIN $8800. */
    ((void(*)(void))0x0100u)();

    /* Execute the overlay, passing a pointer to the game state. */
    ((void(*)(GameState*))OVERLAY_SLOT)(&state);
}

#pragma code-name (pop)
