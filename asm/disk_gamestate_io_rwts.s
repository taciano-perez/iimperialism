; Disk fixed-slot game-state persistence, RWTS experimental backend.
;
; Save/load is intentionally not implemented yet in the RWTS build. The menu
; disables these features, but export the symbols here as explicit stubs so the
; experimental backend does not link against the ProDOS implementation by
; accident.

    .include "disk.inc"

    .export _disk_save_game
    .export _disk_load_game
    .export _disk_read_save_slot_info

    .segment "CODE"

_disk_save_game:
    lda     #DISK_ERR_BACKEND_NOT_READY
    ldx     #$00
    rts

_disk_load_game:
    lda     #DISK_ERR_BACKEND_NOT_READY
    ldx     #$00
    rts

_disk_read_save_slot_info:
    lda     #DISK_ERR_BACKEND_NOT_READY
    ldx     #$00
    rts
