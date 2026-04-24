#include <apple2.h>
#include <em.h>
#include "game.h"
#include "disk.h"
#include "overlay.h"

#pragma code-name (push, "LOWCODE")

#define OVERLAY_CACHE_PAGES   (OVL_COUNT * OVERLAY_PAGES)

static unsigned char overlay_cache_enabled;
static struct em_copy overlay_copy_params;

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

static unsigned overlay_cache_page(unsigned char id) {
    return (unsigned)id * OVERLAY_PAGES;
}

static void cache_overlay_from_slot(unsigned char id) {
    overlay_copy_params.buf = (void*)OVERLAY_SLOT;
    overlay_copy_params.offs = 0U;
    overlay_copy_params.page = overlay_cache_page(id);
    overlay_copy_params.count = OVERLAY_SIZE;
    overlay_copy_params.unused = 0U;
    em_copyto(&overlay_copy_params);
}

static void load_overlay_from_cache(unsigned char id) {
    overlay_copy_params.buf = (void*)OVERLAY_SLOT;
    overlay_copy_params.offs = 0U;
    overlay_copy_params.page = overlay_cache_page(id);
    overlay_copy_params.count = OVERLAY_SIZE;
    overlay_copy_params.unused = 0U;
    em_copyfrom(&overlay_copy_params);
}

void init_overlays(void) {
    unsigned char id;

    overlay_cache_enabled = 0U;

    if (em_install(a2_auxmem_emd) != EM_ERR_OK) {
        return;
    }

    if (em_pagecount() < OVERLAY_CACHE_PAGES) {
        em_uninstall();
        return;
    }

    for (id = 0U; id < OVL_COUNT; ++id) {
        load_overlay_file(id);
        cache_overlay_from_slot(id);
    }

    overlay_cache_enabled = 1U;
}

/* Load overlay <id> into OVERLAY_SLOT ($8800 in MAIN) and execute it. */
void run_overlay(unsigned char id) {
    if (!overlay_label(id)) {
        overlay_load_failed(0, 0, 0);
    }

    if (overlay_cache_enabled) {
        load_overlay_from_cache(id);
    } else {
        load_overlay_file(id);
    }

    /* Execute the overlay. Overlays access game state directly via _state. */
    ((void(*)(void))OVERLAY_SLOT)();
}

#pragma code-name (pop)
