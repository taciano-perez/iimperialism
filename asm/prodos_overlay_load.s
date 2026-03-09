; ProDOS overlay loader using direct MLI OPEN/READ/CLOSE calls.
;
; Exports:
;   _prodos_load_overlay  - __fastcall__ unsigned char(const char* filename)
;   _overlay_bytes_read   - transfer count from the last READ call

    .include "zeropage.inc"

    .export _prodos_load_overlay
    .export _overlay_bytes_read

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
    .addr   PATHNAME
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

_overlay_bytes_read:
    .word   $0000

PATHNAME:
    .res    66

    .segment "LOWCODE"

_prodos_load_overlay:
    sta     ptr1
    stx     ptr1+1

    lda     #$00
    sta     _overlay_bytes_read
    sta     _overlay_bytes_read+1

    ldy     #$00
@copy_filename:
    lda     (ptr1),y
    beq     @filename_done
    sta     PATHNAME+1,y
    iny
    cpy     #64
    bcc     @copy_filename

@filename_done:
    tya
    sta     PATHNAME
    lda     #$00
    sta     PATHNAME+1,y

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
    sta     _overlay_bytes_read
    lda     READ_TRANS+1
    sta     _overlay_bytes_read+1

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
