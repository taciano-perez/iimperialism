#ifndef OVERLAY_H
#define OVERLAY_H

/* Address in MAIN RAM where overlay code is copied before execution.
 * Overlays are 8 pages (2048 bytes) each, so the slot spans $8800-$8FFF. */
#define OVERLAY_SLOT     0x8800
#define OVERLAY_PAGES    8       /* pages per overlay (1 page = 256 bytes) */
#define OVERLAY_SIZE     2048    /* OVERLAY_PAGES * 256 */

/* Overlay IDs used by run_overlay() to select a disk file. */
#define OVL_INDUSTRY   0
#define OVL_PRODUCTION 1
#define OVL_TRANSPORT  2
#define OVL_ADMIRALTY  3
#define OVL_DIPLOMACY 4
#define OVL_TRADE_EXPEDITION 5
#define OVL_TRADE_EXPEDITION_ACTION 6
#define OVL_BATTLE 7
#define OVL_SCIENCE 8

/* Disk file names for the overlay binaries (ProDOS, uppercase, no extension) */
#define OVL_FILE_INDUSTRY   "ISCR"
#define OVL_FILE_PRODUCTION "PSCR"
#define OVL_FILE_TRANSPORT  "TSCR"
#define OVL_FILE_ADMIRALTY  "ASCR"
#define OVL_FILE_DIPLOMACY "DSCR"
#define OVL_FILE_TRADE_EXPEDITION "TEXP"
#define OVL_FILE_TRADE_EXPEDITION_ACTION "TXAC"
#define OVL_FILE_BATTLE "BSCR"
#define OVL_FILE_SCIENCE "SSCR"

/* Overlays are loaded on demand by run_overlay(). */
void init_overlays(void);

/* Load overlay <id> from disk into OVERLAY_SLOT ($8800) and execute it. */
void run_overlay(unsigned char id);

#endif /* OVERLAY_H */
