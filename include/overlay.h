#ifndef OVERLAY_H
#define OVERLAY_H

/* Address in MAIN RAM where overlay code is copied before execution.
 * Overlays are 8 pages (2048 bytes) each, so the slot spans $8800-$8FFF. */
#define OVERLAY_SLOT     0x8800
#define OVERLAY_PAGES    8       /* pages per overlay (1 page = 256 bytes) */
#define OVERLAY_SIZE     2048    /* OVERLAY_PAGES * 256 */

/* Overlay IDs — correspond to AUX RAM layout (8 pages = $0800 bytes apart):
 *   OVL_INDUSTRY   -> AUX $8800-$8FFF
 *   OVL_PRODUCTION -> AUX $9000-$97FF
 *   OVL_TRANSPORT  -> AUX $9800-$9FFF
 */
#define OVL_INDUSTRY   0
#define OVL_PRODUCTION 1
#define OVL_TRANSPORT  2

/* Disk file names for the overlay binaries (ProDOS, uppercase, no extension) */
#define OVL_FILE_INDUSTRY   "ISCR"
#define OVL_FILE_PRODUCTION "PSCR"
#define OVL_FILE_TRANSPORT  "TSCR"

/* Zero-page trampoline parameters ($9A-$9E, outside cc65 ZP range $80-$99).
 * Set by run_overlay before calling the trampoline at $0100. */
#define tramp_src    ((unsigned char*)0x9A)   /* 2 bytes: AUX source lo/hi */
#define tramp_dst    ((unsigned char*)0x9C)   /* 2 bytes: MAIN dest  lo/hi */
#define tramp_pages  (*(unsigned char*)0x9E)  /* page count */

/* Load all overlay binaries from disk into AUX RAM, then install the
 * RAMRD trampoline in the hardware stack page ($0100). Call once at startup
 * before any run_overlay() call. */
void init_overlays(void);

/* Copy overlay <id> from AUX RAM to OVERLAY_SLOT ($8800) and execute it,
 * passing &state as the GameState* argument. */
void run_overlay(unsigned char id);

#endif /* OVERLAY_H */
