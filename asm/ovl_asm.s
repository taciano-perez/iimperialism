; overlay.s — AUX overlay support routines (segment LOWCODE)
;
; Exports:
;   _install_trampoline  — copies RAMRD trampoline into $0100-$0117
;   _main_to_aux         — copies N pages from MAIN to AUX via RAMWRT
;
; Zero-page layout used by both routines and the trampoline ($9A-$9E,
; safely outside cc65's ZP range of $80-$99):
;   $9A/$9B  src address lo/hi
;   $9C/$9D  dst address lo/hi
;   $9E      page count

    .importzp c_sp          ; cc65 software stack pointer (ZP $80/$81)

    .export _install_trampoline
    .export _main_to_aux

    .segment "LOWCODE"

; ---------------------------------------------------------------------------
; void install_trampoline(void)
;
; Copies the 24-byte RAMRD trampoline from TRAMP_CODE into $0100-$0117.
; Must be called once at startup before any run_overlay().
; ---------------------------------------------------------------------------
_install_trampoline:
    ldx #(TRAMP_LEN - 1)
@loop:
    lda TRAMP_CODE,x
    sta $0100,x
    dex
    bpl @loop
    rts

; ---------------------------------------------------------------------------
; Trampoline code image (executed from $0100 with RAMRD active).
;
; When called, ZP $9A-$9E must already be set:
;   $9A/$9B = AUX source address
;   $9C/$9D = MAIN destination address ($8800)
;   $9E     = page count (2 for a 512-byte overlay)
;
; Execution at $0100 is safe with RAMRD on because the hardware stack page
; ($0100-$01FF) is controlled by ALTZP (not RAMRD), so instruction fetches
; always come from MAIN regardless of RAMRD state.
;
; Absolute branch offsets (must match execution at $0100):
;   BNE inner_loop : $010A -> $0105  offset = $F9
;   BNE outer_loop : $0112 -> $0103  offset = $EF
; ---------------------------------------------------------------------------
TRAMP_CODE:
    ; $0100  Enable RAMRD: reads from $0200-$BFFF come from AUX
    .byte $8D, $03, $C0     ; STA $C003
    ; $0103  Reset byte offset
    .byte $A0, $00           ; LDY #0
    ; $0105  Read one byte from AUX (RAMRD on, address in $8800+ range)
    .byte $B1, $9A           ; LDA ($9A),Y
    ; $0107  Write to MAIN (RAMWRT off, so writes go to MAIN)
    .byte $91, $9C           ; STA ($9C),Y
    ; $0109
    .byte $C8                ; INY
    ; $010A  Inner loop: 256 bytes per page (Y wraps 255->0, Z set)
    .byte $D0, $F9           ; BNE $0105
    ; $010C  Advance source page
    .byte $E6, $9B           ; INC $9B
    ; $010E  Advance dest page
    .byte $E6, $9D           ; INC $9D
    ; $0110  Decrement page counter
    .byte $C6, $9E           ; DEC $9E
    ; $0112  Outer loop: repeat for each page
    .byte $D0, $EF           ; BNE $0103
    ; $0114  Disable RAMRD: reads return to MAIN
    .byte $8D, $02, $C0     ; STA $C002
    ; $0117  Return to run_overlay
    .byte $60                ; RTS

TRAMP_LEN = * - TRAMP_CODE     ; 24 bytes

; ---------------------------------------------------------------------------
; void main_to_aux(unsigned int src, unsigned int dst, unsigned char pages)
;
; cc65 default calling convention (3 arguments):
;   On entry: A   = pages (lo)          <- rightmost param passed in A:X
;             X   = 0    (pages hi, ignored)
;             sp+0 = dst_lo
;             sp+1 = dst_hi
;             sp+2 = src_lo
;             sp+3 = src_hi
;   Must add 4 to sp before returning (pop src + dst, each 2 bytes).
;
; Enables RAMWRT so that writes to $0200-$BFFF go to AUX RAM.
; Reads come from MAIN (RAMRD stays off), so the copy loop correctly
; reads from MAIN src and writes to AUX dst.
; All work is done using ZP pointers; the cc65 software stack is
; never written while RAMWRT is active.
; ---------------------------------------------------------------------------
_main_to_aux:
    ; A = pages (rightmost param)
    sta $9E

    ; Load dst from software stack (sp+0/1)
    ldy #0
    lda (c_sp),y
    sta $9C
    iny
    lda (c_sp),y
    sta $9D

    ; Load src from software stack (sp+2/3)
    iny
    lda (c_sp),y
    sta $9A
    iny
    lda (c_sp),y
    sta $9B

    ; Pop 4 bytes from software stack (dst=2 bytes + src=2 bytes)
    lda c_sp
    clc
    adc #4
    sta c_sp
    bcc @nocarry
    inc c_sp+1
@nocarry:

    ; Enable RAMWRT: writes to $0200-$BFFF go to AUX
    ; (A may be garbage here; any write to $C005 enables RAMWRT)
    sta $C005

    ; Copy $9E pages: read from MAIN ($9A/$9B), write to AUX ($9C/$9D)
    ldy #0
@page_loop:
    lda ($9A),y         ; read from MAIN
    sta ($9C),y         ; write to AUX (RAMWRT on)
    iny
    bne @page_loop      ; inner loop: 256 bytes (Y wraps 255->0)
    inc $9B             ; advance source page
    inc $9D             ; advance dest page
    dec $9E             ; decrement page counter
    bne @page_loop      ; outer loop: next page (Y is already 0)

    ; Disable RAMWRT: writes return to MAIN
    sta $C004
    rts
