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
#define OVL_ADMIRALTY_TRADER  4
#define OVL_ADMIRALTY_WARSHIP 5
#define OVL_DIPLOMACY 6
#define OVL_TRADE_EXPEDITION 7
#define OVL_TRADE_EXPEDITION_ACTION 8
#define OVL_BATTLE 9
#define OVL_SCIENCE 10

/* Disk file names for the overlay binaries (ProDOS, uppercase, no extension) */
#define OVL_FILE_INDUSTRY   "ISCR"
#define OVL_FILE_PRODUCTION "PSCR"
#define OVL_FILE_TRANSPORT  "TSCR"
#define OVL_FILE_ADMIRALTY  "ASCR"
#define OVL_FILE_ADMIRALTY_TRADER  "ATRD"
#define OVL_FILE_ADMIRALTY_WARSHIP "AWRS"
#define OVL_FILE_DIPLOMACY "DSCR"
#define OVL_FILE_TRADE_EXPEDITION "TEXP"
#define OVL_FILE_TRADE_EXPEDITION_ACTION "TXAC"
#define OVL_FILE_BATTLE "BSCR"
#define OVL_FILE_SCIENCE "SSCR"

/* Zero-page trampoline parameters ($9A-$9E, outside cc65 ZP range $80-$99).
 * Set by run_overlay before calling the trampoline at $0100. */
#define tramp_src    ((unsigned char*)0x9A)   /* 2 bytes: AUX source lo/hi */
#define tramp_dst    ((unsigned char*)0x9C)   /* 2 bytes: MAIN dest  lo/hi */
#define tramp_pages  (*(unsigned char*)0x9E)  /* page count */

/* Install the RAMRD trampoline in the hardware stack page ($0100).
 * Overlays are loaded on demand by run_overlay(). Call once at startup. */
void init_overlays(void);

/* Load overlay <id> from disk into OVERLAY_SLOT ($8800), mirror it into
 * AUX RAM at the same address, then execute it, passing &state. */
void run_overlay(unsigned char id);

#endif /* OVERLAY_H */
