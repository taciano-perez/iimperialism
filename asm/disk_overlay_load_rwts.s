; Disk overlay loader, RWTS experimental backend.
;
; This backend expects the experimental qboot/ProRWTS boot path to have run
; already. The boot loader leaves ProRWTS relocated at $BD00, which gives us a
; tiny file loader for the resident runtime without keeping ProDOS in memory.

    .include "disk.inc"
    .include "disk_overlay_table.inc"

    .export _disk_get_capabilities
    .export _disk_load_overlay
    .export _disk_overlay_bytes_read

    .segment "DATA"

_disk_overlay_bytes_read:
    .word   $0000

    .segment "LOWCODE"

PRORWTS_STATUS  := $F3
PRORWTS_NAMLO   := $FB
PRORWTS_NAMHI   := $FC
PRORWTS_OPENDIR := $BD00
OVERLAY_SIZE    := $0800

_disk_get_capabilities:
    lda     #$00
    ldx     #$00
    rts

_disk_load_overlay:
    tax

    lda     #$00
    sta     _disk_overlay_bytes_read
    sta     _disk_overlay_bytes_read+1

    cpx     #DISK_OVERLAY_COUNT
    bcc     @valid_overlay
    lda     #DISK_ERR_INVALID_OVERLAY
    ldx     #$00
    rts

@valid_overlay:
    lda     OVL_NAME_LO,x
    sta     PRORWTS_NAMLO
    lda     OVL_NAME_HI,x
    sta     PRORWTS_NAMHI

    jsr     PRORWTS_OPENDIR
    lda     PRORWTS_STATUS
    bne     @error

    lda     #<OVERLAY_SIZE
    sta     _disk_overlay_bytes_read
    lda     #>OVERLAY_SIZE
    sta     _disk_overlay_bytes_read+1

    lda     #$00
    ldx     #$00
    rts

@error:
    ldx     #$00
    rts
