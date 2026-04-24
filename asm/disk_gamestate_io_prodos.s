; Disk fixed-slot game-state persistence, ProDOS backend.
;
; Current backend: ProDOS MLI file operations. Keep the public symbol names
; backend-neutral so higher-level menu code can switch storage backends later.
;
; GAME.DATA is a compact 5-slot container:
;   4 bytes container header ("IS", version, slot count)
;   5 records, each:
;     2 bytes magic ("IG" = $4947)
;     1 byte  save version
;     N bytes serialized GameState payload
;
; With the current 186-byte GameState, the file uses 949 bytes and fits in
; two 512-byte ProDOS blocks. New or invalid containers are explicitly filled
; with zero records so empty slot probes never depend on sparse-file behavior.

    .include "zeropage.inc"

    .export _disk_save_game
    .export _disk_load_game
    .export _disk_read_save_slot_info

    .import _game_state_size
    .import _save_slot

MLI                 := $BF00
CREATE_CALL         := $C0
OPEN_CALL           := $C8
READ_CALL           := $CA
WRITE_CALL          := $CB
CLOSE_CALL          := $CC
SET_MARK_CALL       := $CE
FILE_NOT_FOUND_ERR  := $46
SHORT_READ_ERR      := $FD
SHORT_WRITE_ERR     := $FC
BAD_SAVE_ERR        := $FB
PRODOS_IO_BUFFER    := $8400

SLOT_COUNT          := 5
CONTAINER_SIZE      := 4
RECORD_SIZE         := 189
SAVE_FILE_SIZE      := CONTAINER_SIZE + RECORD_SIZE * SLOT_COUNT
SAVE_HEADER_SIZE    := 3
CONTAINER_VERSION   := 3

; Offsets into the raw GameState payload. These are deliberately duplicated
; here instead of computed by C so the menu can read slot summaries without
; loading an entire 186-byte save into live game state.
TURN_OFFSET         := 150
NATION_NAME_OFFSET  := 157

; Offsets into the small SaveSlotInfo struct declared in ovl_game_menu.c.
INFO_VALID_OFFSET   := 0
INFO_TURN_OFFSET    := 1
INFO_NAME_OFFSET    := 3

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

OPEN_PARAM:
    ; OPEN uses a fixed pathname and a 1KB ProDOS file I/O buffer. Use the
    ; resident free gap at $8400-$87FF: above resident BSS/ONCE, below the
    ; overlay slot at $8800, and well below ProDOS system memory.
    .byte   $03
    .addr   SAVE_PATH
    .addr   PRODOS_IO_BUFFER
OPEN_REF:
    .byte   $00

READ_PARAM:
    ; READ/WRITE/SET_MARK all share the same open reference number, patched
    ; into the respective parameter blocks after OPEN succeeds.
    .byte   $04
READ_REF:
    .byte   $00
READ_ADDR:
    ; Patched with the caller's destination buffer before each read.
    .addr   $0000
READ_COUNT:
    ; Patched with the exact byte count expected by read_chunk.
    .word   $0000
READ_TRANS:
    ; ProDOS reports the actual number of transferred bytes here.
    .word   $0000

WRITE_PARAM:
    .byte   $04
WRITE_REF:
    .byte   $00
WRITE_ADDR:
    ; Patched with the caller's source buffer before each write.
    .addr   $0000
WRITE_COUNT:
    ; Patched with the exact byte count expected by write_chunk.
    .word   $0000
WRITE_TRANS:
    .word   $0000

MARK_PARAM:
    ; Used by SET_MARK. ProDOS expects a 24-bit file offset.
    .byte   $02
MARK_REF:
    .byte   $00
MARK_POS:
    .byte   $00,$00,$00

CLOSE_PARAM:
    ; Shared close block; only the reference number is filled dynamically.
    .byte   $01
CLOSE_REF:
    .byte   $00

CONTAINER_HEADER:
    ; Identifies the multi-slot container independently from per-slot headers.
    .byte   "IS"
    .byte   CONTAINER_VERSION
    .byte   SLOT_COUNT

SAVE_HEADER:
    ; Persisted ahead of every occupied slot's GameState payload.
    .word   $4947
    .byte   $04

HEADER_BUFFER:
    ; Large enough for either the 4-byte container header or 3-byte slot header.
    .res    4

SAVE_PATH:
    .byte   .strlen("GAME.DATA")
    .byte   "GAME.DATA"

SLOT_OFFSET_LO:
    ; Precomputed slot starts avoid multiplying by 189 on the 6502.
    .byte   <(CONTAINER_SIZE + RECORD_SIZE * 0)
    .byte   <(CONTAINER_SIZE + RECORD_SIZE * 1)
    .byte   <(CONTAINER_SIZE + RECORD_SIZE * 2)
    .byte   <(CONTAINER_SIZE + RECORD_SIZE * 3)
    .byte   <(CONTAINER_SIZE + RECORD_SIZE * 4)
SLOT_OFFSET_HI:
    .byte   >(CONTAINER_SIZE + RECORD_SIZE * 0)
    .byte   >(CONTAINER_SIZE + RECORD_SIZE * 1)
    .byte   >(CONTAINER_SIZE + RECORD_SIZE * 2)
    .byte   >(CONTAINER_SIZE + RECORD_SIZE * 3)
    .byte   >(CONTAINER_SIZE + RECORD_SIZE * 4)

ZERO_RECORD:
    ; One zero-filled slot record. init_container writes this five times after
    ; the container header, producing a deterministic 949-byte file.
    .res    RECORD_SIZE

INIT_COUNT:
    .byte   $00

STATE_PTR:
    ; Stable copy of the GameState pointer passed by C. Do not rely on cc65's
    ; ptr1 surviving ProDOS MLI calls.
    .addr   $0000

INFO_PTR:
    ; Stable copy of the SaveSlotInfo pointer passed by C.
    .addr   $0000

    .segment "CODE"

_disk_save_game:
    ; __fastcall__ passes the GameState pointer in A/X.
    sta     STATE_PTR
    stx     STATE_PTR+1

    ; Reuse an existing save container if present. If this is the first save on
    ; the disk, create GAME.DATA and reopen it to obtain a writable refnum.
    jsr     open_file
    bcc     save_open_ok
    cmp     #FILE_NOT_FOUND_ERR
    bne     return_error

    jsr     create_file
    bcs     return_error
    jsr     open_file
    bcs     return_error

save_open_ok:
    ; Copy OPEN_REF into every parameter block used by the save path.
    jsr     set_refs_for_write

    ; If the file is missing, old-format, or from the bad sparse-container
    ; development version, initialize a complete zero-filled container before
    ; writing the selected slot. Valid containers keep their other slots.
    jsr     validate_container_header
    bcc     save_container_ready
    jsr     init_container
    bcs     save_failed

save_container_ready:
    ; Move to the selected slot and write its per-save header followed by the
    ; raw GameState bytes. _save_slot is set by the C menu code.
    jsr     set_mark_slot
    bcs     save_failed

    lda     #<SAVE_HEADER
    sta     WRITE_ADDR
    lda     #>SAVE_HEADER
    sta     WRITE_ADDR+1
    lda     #SAVE_HEADER_SIZE
    sta     WRITE_COUNT
    lda     #$00
    sta     WRITE_COUNT+1
    jsr     write_chunk
    bcs     save_failed

    lda     STATE_PTR
    sta     WRITE_ADDR
    lda     STATE_PTR+1
    sta     WRITE_ADDR+1
    lda     _game_state_size
    sta     WRITE_COUNT
    lda     _game_state_size+1
    sta     WRITE_COUNT+1
    jsr     write_chunk
    bcs     save_failed

    ; Success is reported only after CLOSE succeeds, so ProDOS gets a chance to
    ; flush directory metadata and block state.
    jsr     close_file
    bcs     return_error
    lda     #$00
    tax
    rts

return_error:
    ; Return the current A as the ProDOS/local error code, with X cleared for
    ; the cc65 unsigned char return convention.
    ldx     #$00
    rts

save_failed:
    ; Preserve the original failure code while still attempting to close the
    ; file. CLOSE errors are intentionally ignored here.
    pha
    jsr     close_file
    pla
    ldx     #$00
    rts

_disk_load_game:
    ; __fastcall__ passes the GameState destination pointer in A/X.
    sta     STATE_PTR
    stx     STATE_PTR+1

    ; Loading never creates the file. Missing or invalid containers surface as
    ; load failure to the menu.
    jsr     open_file
    bcs     return_error
    jsr     set_refs_for_read

    ; Validate both the container header and the selected slot's header before
    ; mutating live game state. Empty slots fail read_save_header.
    jsr     validate_container_header
    bcs     load_failed
    jsr     set_mark_slot
    bcs     load_failed
    jsr     read_save_header
    bcs     load_failed

    lda     STATE_PTR
    sta     READ_ADDR
    lda     STATE_PTR+1
    sta     READ_ADDR+1
    lda     _game_state_size
    sta     READ_COUNT
    lda     _game_state_size+1
    sta     READ_COUNT+1
    jsr     read_chunk
    bcs     load_failed

    ; As with saving, report success only after the ProDOS file is closed.
    jsr     close_file
    bcs     return_error
    lda     #$00
    tax
    rts

load_failed:
    pha
    jsr     close_file
    pla
    ldx     #$00
    rts

_disk_read_save_slot_info:
    ; __fastcall__ passes the SaveSlotInfo pointer in A/X.
    sta     INFO_PTR
    stx     INFO_PTR+1

    ; Default the caller's info.valid to false. Every failure path below leaves
    ; the slot marked empty, including missing file, old single-save format,
    ; invalid container header, short reads, and empty slot records.
    lda     INFO_PTR
    sta     ptr1
    lda     INFO_PTR+1
    sta     ptr1+1
    ldy     #INFO_VALID_OFFSET
    lda     #$00
    sta     (ptr1),y

    ; Missing save file is not an error for listing slots; it simply means all
    ; slots are empty.
    jsr     open_file
    bcc     info_open_ok
    cmp     #FILE_NOT_FOUND_ERR
    bne     return_error
    lda     #$00
    tax
    rts

info_open_ok:
    jsr     set_refs_for_read

    ; Container-level and disk-position/read errors are now returned to the menu
    ; so it can show "I/O Error XX" instead of repeatedly probing the disk.
    jsr     validate_container_header
    bcs     info_failed
    jsr     set_mark_slot
    bcs     info_failed
    jsr     read_save_header
    bcc     info_slot_has_header
    cmp     #BAD_SAVE_ERR
    bne     info_failed
    beq     info_empty

info_slot_has_header:
    ; Read turn_number directly from the selected slot's GameState payload into
    ; SaveSlotInfo.turn_number.
    jsr     set_mark_slot_turn
    bcs     info_failed
    lda     INFO_PTR
    clc
    adc     #INFO_TURN_OFFSET
    sta     READ_ADDR
    lda     INFO_PTR+1
    adc     #$00
    sta     READ_ADDR+1
    lda     #$02
    sta     READ_COUNT
    lda     #$00
    sta     READ_COUNT+1
    jsr     read_chunk
    bcs     info_failed

    ; Read nation_name directly from the selected slot's GameState payload into
    ; SaveSlotInfo.nation_name. The source field is already NUL-terminated by
    ; the game state initialization/input path.
    jsr     set_mark_slot_name
    bcs     info_failed
    lda     INFO_PTR
    clc
    adc     #INFO_NAME_OFFSET
    sta     READ_ADDR
    lda     INFO_PTR+1
    adc     #$00
    sta     READ_ADDR+1
    lda     #$14
    sta     READ_COUNT
    lda     #$00
    sta     READ_COUNT+1
    jsr     read_chunk
    bcs     info_failed

    ; Only mark the slot valid after both metadata reads completed.
    lda     INFO_PTR
    sta     ptr1
    lda     INFO_PTR+1
    sta     ptr1+1
    ldy     #INFO_VALID_OFFSET
    lda     #$01
    sta     (ptr1),y

info_empty:
    ; Close if OPEN succeeded. The metadata API intentionally returns success
    ; for invalid/empty slots so the menu can keep drawing the list.
    jsr     close_file
    bcc     success
    ldx     #$00
    rts

info_failed:
    ; Preserve the ProDOS/local error code, close the file, then return it.
    pha
    jsr     close_file
    pla
    ldx     #$00
    rts

success:
    lda     #$00
    tax
    rts

set_refs_for_write:
    ; OPEN writes one refnum; each MLI parameter block needs its own copy.
    lda     OPEN_REF
    sta     WRITE_REF
    sta     MARK_REF
    sta     CLOSE_REF
    rts

set_refs_for_read:
    ; Read paths need READ, SET_MARK, and CLOSE to address the same open file.
    lda     OPEN_REF
    sta     READ_REF
    sta     MARK_REF
    sta     CLOSE_REF
    rts

create_file:
    ; CREATE allocates the directory entry only. Callers must still OPEN before
    ; doing READ/WRITE/SET_MARK operations.
    jsr     MLI
    .byte   CREATE_CALL
    .word   CREATE_PARAM
    rts

open_file:
    ; OPEN fills OPEN_REF on success.
    jsr     MLI
    .byte   OPEN_CALL
    .word   OPEN_PARAM
    rts

close_file:
    ; CLOSE uses whichever refnum was copied into CLOSE_REF by set_refs_*.
    jsr     MLI
    .byte   CLOSE_CALL
    .word   CLOSE_PARAM
    rts

set_mark_start:
    ; Position at byte 0, used for reading/writing the container header.
    lda     #$00
    sta     MARK_POS
    sta     MARK_POS+1
    sta     MARK_POS+2
    jmp     set_mark

set_mark_slot:
    ; Position at the start of the selected record:
    ;   CONTAINER_SIZE + _save_slot * RECORD_SIZE
    ldx     _save_slot
    lda     SLOT_OFFSET_LO,x
    sta     MARK_POS
    lda     SLOT_OFFSET_HI,x
    sta     MARK_POS+1
    lda     #$00
    sta     MARK_POS+2
    jmp     set_mark

set_mark_slot_turn:
    ; Position at selected slot's GameState.turn_number field. This is used by
    ; the menu list and does not disturb live game state.
    ldx     _save_slot
    lda     SLOT_OFFSET_LO,x
    clc
    adc     #(SAVE_HEADER_SIZE + TURN_OFFSET)
    sta     MARK_POS
    lda     SLOT_OFFSET_HI,x
    adc     #$00
    sta     MARK_POS+1
    lda     #$00
    sta     MARK_POS+2
    jmp     set_mark

set_mark_slot_name:
    ; Position at selected slot's GameState.nation_name field for menu display.
    ldx     _save_slot
    lda     SLOT_OFFSET_LO,x
    clc
    adc     #(SAVE_HEADER_SIZE + NATION_NAME_OFFSET)
    sta     MARK_POS
    lda     SLOT_OFFSET_HI,x
    adc     #$00
    sta     MARK_POS+1
    lda     #$00
    sta     MARK_POS+2
    jmp     set_mark

set_mark:
    ; Shared SET_MARK call. MARK_POS is loaded by the helper immediately before
    ; jumping here.
    jsr     MLI
    .byte   SET_MARK_CALL
    .word   MARK_PARAM
    rts

init_container:
    ; Build the whole 949-byte save container by writing:
    ;   4-byte container header
    ;   5 zero-filled 189-byte slot records
    ; This makes all unsaved slots real bytes on disk and avoids hanging while
    ; probing unwritten space on some ProDOS/emulator combinations.
    jsr     set_mark_start
    bcs     init_error

    lda     #<CONTAINER_HEADER
    sta     WRITE_ADDR
    lda     #>CONTAINER_HEADER
    sta     WRITE_ADDR+1
    lda     #CONTAINER_SIZE
    sta     WRITE_COUNT
    lda     #$00
    sta     WRITE_COUNT+1
    jsr     write_chunk
    bcs     init_error

    lda     #SLOT_COUNT
    sta     INIT_COUNT

@zero_slot:
    lda     #<ZERO_RECORD
    sta     WRITE_ADDR
    lda     #>ZERO_RECORD
    sta     WRITE_ADDR+1
    lda     #<RECORD_SIZE
    sta     WRITE_COUNT
    lda     #>RECORD_SIZE
    sta     WRITE_COUNT+1
    jsr     write_chunk
    bcs     init_error

    dec     INIT_COUNT
    bne     @zero_slot
    clc
    rts

init_error:
    rts

validate_container_header:
    ; Rewind and read the 4-byte multi-slot container header.
    jsr     set_mark_start
    bcs     validate_error

    lda     #<HEADER_BUFFER
    sta     READ_ADDR
    lda     #>HEADER_BUFFER
    sta     READ_ADDR+1
    lda     #CONTAINER_SIZE
    sta     READ_COUNT
    lda     #$00
    sta     READ_COUNT+1
    jsr     read_chunk
    bcs     validate_error

    ; Magic/version/slot-count all have to match. This deliberately rejects the
    ; older one-save GAME.DATA format.
    lda     HEADER_BUFFER
    cmp     #'I'
    bne     validate_bad
    lda     HEADER_BUFFER+1
    cmp     #'S'
    bne     validate_bad
    lda     HEADER_BUFFER+2
    cmp     #CONTAINER_VERSION
    bne     validate_bad
    lda     HEADER_BUFFER+3
    cmp     #SLOT_COUNT
    bne     validate_bad
    clc
    rts

validate_bad:
    ; Local validation error distinct from ProDOS errors.
    lda     #BAD_SAVE_ERR
    sec
validate_error:
    rts

read_save_header:
    ; Read and validate the 3-byte per-slot header. Unsaved slots are still
    ; zero-filled, so they fail here and are displayed as <Empty>.
    lda     #<HEADER_BUFFER
    sta     READ_ADDR
    lda     #>HEADER_BUFFER
    sta     READ_ADDR+1
    lda     #SAVE_HEADER_SIZE
    sta     READ_COUNT
    lda     #$00
    sta     READ_COUNT+1
    jsr     read_chunk
    bcs     read_header_error

    lda     HEADER_BUFFER
    cmp     #<$4947
    bne     read_header_bad
    lda     HEADER_BUFFER+1
    cmp     #>$4947
    bne     read_header_bad
    lda     HEADER_BUFFER+2
    cmp     #$04
    bne     read_header_bad
    clc
    rts

read_header_bad:
    ; Local validation error distinct from ProDOS errors.
    lda     #BAD_SAVE_ERR
    sec
read_header_error:
    rts

read_chunk:
    ; Treat short reads as failure even if MLI itself succeeded. The save format
    ; is fixed-size and every caller expects the exact requested byte count.
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
    ; Likewise, require the entire requested write to complete. A partial write
    ; leaves the slot invalid/incomplete and must surface as a save failure.
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
