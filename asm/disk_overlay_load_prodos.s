; Disk overlay loader, ProDOS backend.
;
; Current backend: ProDOS MLI OPEN/READ/CLOSE. Keep the public symbol names
; backend-neutral so the higher-level game code does not depend on ProDOS.
;
; Exports:
;   _disk_get_capabilities   - __fastcall__ unsigned char(void)
;   _disk_load_overlay       - __fastcall__ unsigned char(unsigned char id)
;   _disk_overlay_bytes_read - transfer count from the last READ call

    .include "zeropage.inc"
    .include "disk.inc"
    .include "disk_overlay_table.inc"

    .export _disk_get_capabilities
    .export _disk_load_overlay
    .export _disk_overlay_bytes_read

MLI                 := $BF00
OPEN_CALL           := $C8
READ_CALL           := $CA
CLOSE_CALL          := $CC
OVERLAY_SLOT        := $8800
OVERLAY_SIZE        := $0800
PRODOS_IO_BUFFER    := MLI - 1024

    .segment "DATA"

OPEN_PARAM:
    .byte   $03
OPEN_PATH:
    .addr   $0000
    .addr   PRODOS_IO_BUFFER
OPEN_REF:
    .byte   $00

READ_PARAM:
    .byte   $04
READ_REF:
    .byte   $00
    .addr   OVERLAY_SLOT
    .word   OVERLAY_SIZE
READ_TRANS:
    .word   $0000

CLOSE_PARAM:
    .byte   $01
CLOSE_REF:
    .byte   $00

_disk_overlay_bytes_read:
    .word   $0000

    .segment "LOWCODE"

_disk_get_capabilities:
    lda     #DISK_CAP_SAVE_LOAD
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
    sta     OPEN_PATH
    lda     OVL_NAME_HI,x
    sta     OPEN_PATH+1

    jsr     MLI
    .byte   OPEN_CALL
    .word   OPEN_PARAM
    bcc     @open_ok
    ldx     #$00
    rts

@open_ok:
    lda     OPEN_REF
    sta     READ_REF
    sta     CLOSE_REF

    jsr     MLI
    .byte   READ_CALL
    .word   READ_PARAM

    lda     READ_TRANS
    sta     _disk_overlay_bytes_read
    lda     READ_TRANS+1
    sta     _disk_overlay_bytes_read+1

    bcc     @read_ok
    pha
    jsr     @close_file
    pla
    ldx     #$00
    rts

@read_ok:
    jsr     @close_file
    bcc     @success
    ldx     #$00
    rts

@success:
    lda     #$00
    tax
    rts

@close_file:
    jsr     MLI
    .byte   CLOSE_CALL
    .word   CLOSE_PARAM
    rts
