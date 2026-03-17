; ProDOS game-state persistence using direct MLI calls.
;
; This avoids linking cc65 stdio file I/O into resident memory. The file format
; is a tiny fixed header followed by the raw GameState bytes:
;   2 bytes magic  ("IG" = $4947)
;   1 byte  version
;   N bytes serialized GameState payload
;
; Exports:
;   _prodos_save_game  - __fastcall__ unsigned char(const GameState* state)
;   _prodos_load_game  - __fastcall__ unsigned char(GameState* state)
;
; Return convention:
;   A = 0 on success
;   A = ProDOS error code or local validation error on failure
;   X = 0 in both cases

    .include "zeropage.inc"

    .export _prodos_save_game
    .export _prodos_load_game

    .import _game_state_size

MLI                 := $BF00
CREATE_CALL         := $C0
OPEN_CALL           := $C8
READ_CALL           := $CA
WRITE_CALL          := $CB
CLOSE_CALL          := $CC
FILE_NOT_FOUND_ERR  := $46
SHORT_READ_ERR      := $FD
SHORT_WRITE_ERR     := $FC
BAD_SAVE_ERR        := $FB
PRODOS_IO_BUFFER    := MLI - 1024

    .segment "DATA"

; CREATE parameter block for GAME.DATA. We create a BIN file and let ProDOS
; populate metadata such as timestamps and blocks-used.
CREATE_PARAM:
    .byte   $07
    .addr   SAVE_PATH
    .byte   $C3
    .byte   $06
    .word   $0000
    .byte   $01
    .word   $0000
    .word   $0000

; OPEN uses a fixed pathname and the standard I/O buffer just below MLI.
OPEN_PARAM:
    .byte   $03
    .addr   SAVE_PATH
    .addr   PRODOS_IO_BUFFER
OPEN_REF:
    .byte   $00

; READ/WRITE blocks are patched at runtime with the caller's buffer pointer and
; the exact byte count expected for the transfer.
READ_PARAM:
    .byte   $04
READ_REF:
    .byte   $00
READ_ADDR:
    .addr   $0000
READ_COUNT:
    .word   $0000
READ_TRANS:
    .word   $0000

WRITE_PARAM:
    .byte   $04
WRITE_REF:
    .byte   $00
WRITE_ADDR:
    .addr   $0000
WRITE_COUNT:
    .word   $0000
WRITE_TRANS:
    .word   $0000

; Shared close block; only the reference number is filled dynamically.
CLOSE_PARAM:
    .byte   $01
CLOSE_REF:
    .byte   $00

; Persisted header written ahead of the raw GameState bytes.
SAVE_HEADER:
    .word   $4947
    .byte   $03

HEADER_BUFFER:
    .res    3

SAVE_PATH:
    .byte   .strlen("GAME.DATA")
    .byte   "GAME.DATA"

    .segment "LOWCODE"

_prodos_save_game:
    ; __fastcall__ passes the pointer argument in A/X.
    sta     ptr1
    stx     ptr1+1

    ; If the save file does not exist yet, create it and reopen to get a
    ; writable reference number.
    jsr     open_file
    bcc     save_open_ok
    cmp     #FILE_NOT_FOUND_ERR
    bne     return_error

    jsr     create_file
    bcs     return_error
    jsr     open_file
    bcs     return_error

save_open_ok:
    ; OPEN returns the reference number needed by WRITE and CLOSE.
    lda     OPEN_REF
    sta     WRITE_REF
    sta     CLOSE_REF

    ; Write the 3-byte header first.
    lda     #<SAVE_HEADER
    sta     WRITE_ADDR
    lda     #>SAVE_HEADER
    sta     WRITE_ADDR+1
    lda     #$03
    sta     WRITE_COUNT
    lda     #$00
    sta     WRITE_COUNT+1
    jsr     write_chunk
    bcs     save_failed

    ; Then write the full GameState payload.
    lda     ptr1
    sta     WRITE_ADDR
    lda     ptr1+1
    sta     WRITE_ADDR+1
    lda     _game_state_size
    sta     WRITE_COUNT
    lda     _game_state_size+1
    sta     WRITE_COUNT+1
    jsr     write_chunk
    bcs     save_failed

    jsr     close_file
    bcc     success

return_error:
    ldx     #$00
    rts

save_failed:
    ; Preserve the original failure code while still attempting to close.
    pha
    jsr     close_file
    pla
    ldx     #$00
    rts

_prodos_load_game:
    sta     ptr1
    stx     ptr1+1

    ; OPEN failure is returned directly to the caller.
    jsr     open_file
    bcs     return_error

    ; OPEN returns the reference number needed by READ and CLOSE.
    lda     OPEN_REF
    sta     READ_REF
    sta     CLOSE_REF

    ; Read and validate the fixed header before mutating the live game state.
    lda     #<HEADER_BUFFER
    sta     READ_ADDR
    lda     #>HEADER_BUFFER
    sta     READ_ADDR+1
    lda     #$03
    sta     READ_COUNT
    lda     #$00
    sta     READ_COUNT+1
    jsr     read_chunk
    bcs     load_failed

    lda     HEADER_BUFFER
    cmp     #<$4947
    bne     bad_save
    lda     HEADER_BUFFER+1
    cmp     #>$4947
    bne     bad_save
    lda     HEADER_BUFFER+2
    cmp     #$03
    bne     bad_save

    ; Header is valid, so read the exact serialized GameState payload.
    lda     ptr1
    sta     READ_ADDR
    lda     ptr1+1
    sta     READ_ADDR+1
    lda     _game_state_size
    sta     READ_COUNT
    lda     _game_state_size+1
    sta     READ_COUNT+1
    jsr     read_chunk
    bcs     load_failed

    jsr     close_file
    bcc     success
    bcs     return_error

bad_save:
    ; File exists, but its magic or version does not match.
    lda     #BAD_SAVE_ERR

load_failed:
    pha
    jsr     close_file
    pla
    ldx     #$00
    rts

success:
    lda     #$00
    tax
    rts

create_file:
    ; CREATE only allocates the file; callers must still OPEN it.
    jsr     MLI
    .byte   CREATE_CALL
    .word   CREATE_PARAM
    rts

open_file:
    jsr     MLI
    .byte   OPEN_CALL
    .word   OPEN_PARAM
    rts

close_file:
    jsr     MLI
    .byte   CLOSE_CALL
    .word   CLOSE_PARAM
    rts

read_chunk:
    ; Treat short reads as failure even if MLI itself succeeded, because the
    ; save format requires exact byte counts.
    jsr     MLI
    .byte   READ_CALL
    .word   READ_PARAM
    bcs     read_error

    lda     READ_TRANS
    cmp     READ_COUNT
    bne     short_read
    lda     READ_TRANS+1
    cmp     READ_COUNT+1
    beq     read_ok

short_read:
    lda     #SHORT_READ_ERR
    sec
    rts

read_error:
    sec
    rts

read_ok:
    clc
    rts

write_chunk:
    ; Likewise, require the entire requested write to complete.
    jsr     MLI
    .byte   WRITE_CALL
    .word   WRITE_PARAM
    bcs     write_error

    lda     WRITE_TRANS
    cmp     WRITE_COUNT
    bne     short_write
    lda     WRITE_TRANS+1
    cmp     WRITE_COUNT+1
    beq     write_ok

short_write:
    lda     #SHORT_WRITE_ERR
    sec
    rts

write_error:
    sec
    rts

write_ok:
    clc
    rts
