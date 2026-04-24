; Disk fixed-slot game-state persistence, RWTS backend.
;
; This backend uses the write-capable ProRWTS build left resident by the RWTS
; bootstrap. Because ProRWTS writes fixed 256-byte pages on floppy and cannot
; create/resize files, GAME.DATA is preallocated on the disk image as a fixed
; 1024-byte container. The menu overlay reads the full container into hidden
; text-page memory ($0400-$07FF), updates one slot, and writes the whole image
; back out.

    .include "zeropage.inc"
    .include "disk.inc"

    .export _disk_save_game
    .export _disk_load_game
    .export _disk_read_save_slot_info

    .import _game_state_size
    .import _save_slot

PRORWTS_OPENDIR := $BC00
PRORWTS_STATUS  := $F3
PRORWTS_SIZELO  := $F5
PRORWTS_SIZEHI  := $F6
PRORWTS_REQCMD  := $F8
PRORWTS_LDRLO   := $F9
PRORWTS_LDRHI   := $FA
PRORWTS_NAMLO   := $FB
PRORWTS_NAMHI   := $FC

PRORWTS_CMDREAD := $01
PRORWTS_CMDWRITE := $02

RWTS_SAVE_BUFFER := $0400
RWTS_SAVE_SIZE   := $0400

SLOT_COUNT          := 5
CONTAINER_SIZE      := 4
RECORD_SIZE         := 189
SAVE_HEADER_SIZE    := 3
CONTAINER_VERSION   := 3
SAVE_HEADER_VERSION := 4

TURN_OFFSET         := 150
NATION_NAME_OFFSET  := 157

INFO_VALID_OFFSET   := 0
INFO_TURN_OFFSET    := 1
INFO_NAME_OFFSET    := 3

BAD_SAVE_ERR        := $FB

    .segment "DATA"

SAVE_PATH:
    .byte   .strlen("GAME.DATA")
    .byte   "GAME.DATA"

SLOT_OFFSET_LO:
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

STATE_PTR:
    .addr   $0000

INFO_PTR:
    .addr   $0000

    .segment "CODE"

_disk_save_game:
    sta     STATE_PTR
    stx     STATE_PTR+1

    jsr     read_container
    bcs     return_error
    jsr     validate_container_header
    bcc     @container_ok
    jsr     init_container

@container_ok:
    jsr     prepare_slot_ptr

    lda     #<$4947
    ldy     #$00
    sta     (ptr1),y
    iny
    lda     #>$4947
    sta     (ptr1),y
    iny
    lda     #SAVE_HEADER_VERSION
    sta     (ptr1),y

    lda     ptr1
    clc
    adc     #SAVE_HEADER_SIZE
    sta     ptr2
    lda     ptr1+1
    adc     #$00
    sta     ptr2+1

    lda     STATE_PTR
    sta     ptr3
    lda     STATE_PTR+1
    sta     ptr3+1

    lda     _game_state_size
    sta     tmp1
    lda     _game_state_size+1
    sta     tmp2
    jsr     copy_state_to_buffer

    jsr     write_container
    bcs     return_error
    jmp     success

return_error:
    ldx     #$00
    rts

_disk_load_game:
    sta     STATE_PTR
    stx     STATE_PTR+1

    jsr     read_container
    bcs     return_error
    jsr     validate_container_header
    bcs     return_error
    jsr     prepare_slot_ptr
    jsr     validate_save_header
    bcs     return_error

    lda     ptr1
    clc
    adc     #SAVE_HEADER_SIZE
    sta     ptr2
    lda     ptr1+1
    adc     #$00
    sta     ptr2+1

    lda     STATE_PTR
    sta     ptr3
    lda     STATE_PTR+1
    sta     ptr3+1

    lda     _game_state_size
    sta     tmp1
    lda     _game_state_size+1
    sta     tmp2
    jsr     copy_buffer_to_state
    bcs     return_error
    jmp     success

_disk_read_save_slot_info:
    sta     INFO_PTR
    stx     INFO_PTR+1

    lda     INFO_PTR
    sta     ptr1
    lda     INFO_PTR+1
    sta     ptr1+1
    ldy     #INFO_VALID_OFFSET
    lda     #$00
    sta     (ptr1),y

    jsr     read_container
    bcs     return_error
    jsr     validate_container_header
    bcs     return_error
    jsr     prepare_slot_ptr
    jsr     validate_save_header
    bcs     @empty_slot

    lda     ptr1
    clc
    adc     #(SAVE_HEADER_SIZE + TURN_OFFSET)
    sta     ptr2
    lda     ptr1+1
    adc     #$00
    sta     ptr2+1

    lda     INFO_PTR
    clc
    adc     #INFO_TURN_OFFSET
    sta     ptr3
    lda     INFO_PTR+1
    adc     #$00
    sta     ptr3+1

    ldy     #$00
@copy_turn:
    lda     (ptr2),y
    sta     (ptr3),y
    iny
    cpy     #$02
    bcc     @copy_turn

    lda     ptr1
    clc
    adc     #(SAVE_HEADER_SIZE + NATION_NAME_OFFSET)
    sta     ptr2
    lda     ptr1+1
    adc     #$00
    sta     ptr2+1

    lda     INFO_PTR
    clc
    adc     #INFO_NAME_OFFSET
    sta     ptr3
    lda     INFO_PTR+1
    adc     #$00
    sta     ptr3+1

    ldy     #$00
@copy_name:
    lda     (ptr2),y
    sta     (ptr3),y
    iny
    cpy     #20
    bcc     @copy_name

    lda     INFO_PTR
    sta     ptr1
    lda     INFO_PTR+1
    sta     ptr1+1
    ldy     #INFO_VALID_OFFSET
    lda     #$01
    sta     (ptr1),y

@empty_slot:
    lda     #$00
    tax
    rts

success:
    lda     #$00
    tax
    rts

read_container:
    lda     #PRORWTS_CMDREAD
    sta     PRORWTS_REQCMD
    lda     #<RWTS_SAVE_BUFFER
    sta     PRORWTS_LDRLO
    lda     #>RWTS_SAVE_BUFFER
    sta     PRORWTS_LDRHI
    lda     #<SAVE_PATH
    sta     PRORWTS_NAMLO
    lda     #>SAVE_PATH
    sta     PRORWTS_NAMHI
    jsr     PRORWTS_OPENDIR
    lda     PRORWTS_STATUS
    beq     @ok
    sec
    rts
@ok:
    clc
    rts

write_container:
    lda     #$00
    sta     PRORWTS_SIZELO
    lda     #$04
    sta     PRORWTS_SIZEHI
    lda     #PRORWTS_CMDWRITE
    sta     PRORWTS_REQCMD
    lda     #<RWTS_SAVE_BUFFER
    sta     PRORWTS_LDRLO
    lda     #>RWTS_SAVE_BUFFER
    sta     PRORWTS_LDRHI
    lda     #<SAVE_PATH
    sta     PRORWTS_NAMLO
    lda     #>SAVE_PATH
    sta     PRORWTS_NAMHI
    jsr     PRORWTS_OPENDIR
    lda     PRORWTS_STATUS
    beq     @ok
    sec
    rts
@ok:
    clc
    rts

validate_container_header:
    lda     RWTS_SAVE_BUFFER
    cmp     #'I'
    bne     @bad
    lda     RWTS_SAVE_BUFFER+1
    cmp     #'S'
    bne     @bad
    lda     RWTS_SAVE_BUFFER+2
    cmp     #CONTAINER_VERSION
    bne     @bad
    lda     RWTS_SAVE_BUFFER+3
    cmp     #SLOT_COUNT
    bne     @bad
    clc
    rts
@bad:
    lda     #BAD_SAVE_ERR
    sec
    rts

validate_save_header:
    ldy     #$00
    lda     (ptr1),y
    cmp     #<$4947
    bne     @bad
    iny
    lda     (ptr1),y
    cmp     #>$4947
    bne     @bad
    iny
    lda     (ptr1),y
    cmp     #SAVE_HEADER_VERSION
    bne     @bad
    clc
    rts
@bad:
    lda     #BAD_SAVE_ERR
    sec
    rts

init_container:
    lda     #<RWTS_SAVE_BUFFER
    sta     ptr1
    lda     #>RWTS_SAVE_BUFFER
    sta     ptr1+1
    ldx     #$04
    lda     #$00
@clear_page:
    ldy     #$00
@clear_loop:
    sta     (ptr1),y
    iny
    bne     @clear_loop
    inc     ptr1+1
    dex
    bne     @clear_page

    lda     #'I'
    sta     RWTS_SAVE_BUFFER
    lda     #'S'
    sta     RWTS_SAVE_BUFFER+1
    lda     #CONTAINER_VERSION
    sta     RWTS_SAVE_BUFFER+2
    lda     #SLOT_COUNT
    sta     RWTS_SAVE_BUFFER+3

    lda     #<RWTS_SAVE_BUFFER+CONTAINER_SIZE
    sta     ptr1
    lda     #>RWTS_SAVE_BUFFER+CONTAINER_SIZE
    sta     ptr1+1
    ldx     #SLOT_COUNT
@slot_loop:
    ldy     #$00
    lda     #$00
    sta     (ptr1),y
    iny
    sta     (ptr1),y
    iny
    lda     #SAVE_HEADER_VERSION
    sta     (ptr1),y
    lda     ptr1
    clc
    adc     #<RECORD_SIZE
    sta     ptr1
    lda     ptr1+1
    adc     #>RECORD_SIZE
    sta     ptr1+1
    dex
    bne     @slot_loop
    rts

prepare_slot_ptr:
    ldx     _save_slot
    lda     #<RWTS_SAVE_BUFFER
    clc
    adc     SLOT_OFFSET_LO,x
    sta     ptr1
    lda     #>RWTS_SAVE_BUFFER
    adc     SLOT_OFFSET_HI,x
    sta     ptr1+1
    rts

copy_state_to_buffer:
    ldy     #$00
@loop_low:
    lda     (ptr3),y
    sta     (ptr2),y
    iny
    cpy     tmp1
    bcc     @loop_low
    lda     tmp2
    beq     @done
    inc     ptr2+1
    inc     ptr3+1
    dec     tmp2
    ldy     #$00
    jmp     @loop_low
@done:
    rts

copy_buffer_to_state:
    ldy     #$00
@loop_low:
    lda     (ptr2),y
    sta     (ptr3),y
    iny
    cpy     tmp1
    bcc     @loop_low
    lda     tmp2
    beq     @done
    inc     ptr2+1
    inc     ptr3+1
    dec     tmp2
    ldy     #$00
    jmp     @loop_low
@done:
    clc
    rts
