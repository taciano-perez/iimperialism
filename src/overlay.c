#include "game.h"
#include "disk.h"
#include "overlay.h"

#pragma code-name (push, "LOWCODE")

static const char* overlay_label(unsigned char id) {
    switch (id) {
        case OVL_INDUSTRY:   return OVL_FILE_INDUSTRY;
        case OVL_PRODUCTION: return OVL_FILE_PRODUCTION;
        case OVL_TRANSPORT:  return OVL_FILE_TRANSPORT;
        case OVL_ADMIRALTY:  return OVL_FILE_ADMIRALTY;
        case OVL_DIPLOMACY:  return OVL_FILE_DIPLOMACY;
        case OVL_TRADE_EXPEDITION_ACTION: return OVL_FILE_TRADE_EXPEDITION_ACTION;
        case OVL_BATTLE:     return OVL_FILE_BATTLE;
        case OVL_SCIENCE:    return OVL_FILE_SCIENCE;
        case OVL_GAME_MENU:  return OVL_FILE_GAME_MENU;
        case OVL_COUNCIL_NATIONS: return OVL_FILE_COUNCIL_NATIONS;
        default:             return 0;
    }
}

static void overlay_load_failed(const char* filename, const char* reason, unsigned int detail) {
    clear_screen();
    print(1, 1, "Ovl ld fail");
    if (filename) {
        print(1, 3, filename);
    } else {
        print(1, 3, "Inval ovl");
    }
    if (reason) {
        print(1, 5, reason);
        print_int(1, 6, detail);
    }
    while (1) {
    }
}

/* Load one overlay binary from disk into OVERLAY_SLOT (MAIN $8800). */
static void load_overlay_file(unsigned char id) {
    unsigned char error_code;
    const char* label;

    label = overlay_label(id);

    error_code = disk_load_overlay(id);
    if (error_code != 0) {
        overlay_load_failed(label, "Disk err", (unsigned int)error_code);
    }

    if (disk_overlay_bytes_read != OVERLAY_SIZE) {
        overlay_load_failed(label, "read bytes", disk_overlay_bytes_read);
    }
}

void init_overlays(void) {
}

/* Load overlay <id> into OVERLAY_SLOT ($8800 in MAIN) and execute it. */
void run_overlay(unsigned char id) {
    if (!overlay_label(id)) {
        overlay_load_failed(0, 0, 0);
    }

    load_overlay_file(id);

    /* Execute the overlay. Overlays access game state directly via _state. */
    ((void(*)(void))OVERLAY_SLOT)();
}

#pragma code-name (pop)
