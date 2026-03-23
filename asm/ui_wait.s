    .export _wait_three_seconds_or_keypress

    .segment "LOWCODE"

KEYBOARD        := $C000
KEYBOARD_STROBE := $C010
OUTER_LOOPS     := $07

; void wait_three_seconds_or_keypress(void);
; Approximate 3-second delay on a 1 MHz Apple II, returning early if any key is
; already waiting. Reading KEYBOARD_STROBE consumes the pending key so it does
; not leak into the next input prompt.
_wait_three_seconds_or_keypress:
    ldx     #OUTER_LOOPS

outer_loop:
    ldy     #$FF

middle_loop:
    lda     KEYBOARD
    bmi     key_pressed

    lda     #$FF

inner_loop:
    sec
    sbc     #$01
    bne     inner_loop

    dey
    bne     middle_loop

    dex
    bne     outer_loop

    rts

key_pressed:
    bit     KEYBOARD_STROBE
    rts
