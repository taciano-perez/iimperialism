    .export _play_sound_impl

; Apple II hardware
SPKR    := $C030        ; soft switch: reading toggles speaker

; -----------------------------------------------------------------------
; Sound engine ported from Don Lancaster's RM-4 "Obnoxious Sounds" (1982)
; Generates 16 different Apple II speaker effects.
;
; void __fastcall__ play_sound_impl(unsigned char sound);
;   sound arrives in A register (cc65 __fastcall__ convention).
;   X is clobbered; A, Y, processor flags are NOT guaranteed preserved.
; -----------------------------------------------------------------------

    .segment "LOWCODE"

; Sound effect table: pairs of (trip_count, sweep_value).
; trip_count : how many full sweeps to perform (1–255).
; sweep_value: starting pitch/cycle count; sweeps down to 1 each trip.
;              Values >= $80 enable the GEIGER mode (early inner-loop exit).
snd_table:
    .byte $01,$08   ;  0  TICK
    .byte $01,$18   ;  1  WHOPIDOOP
    .byte $FF,$01   ;  2  PIP
    .byte $06,$10   ;  3  PHASOR
    .byte $01,$30   ;  4  MUSIC SCALE
    .byte $20,$06   ;  5  SHORT BRASS
    .byte $70,$06   ;  6  MEDIUM BRASS
    .byte $FF,$06   ;  7  LONG BRASS
    .byte $01,$A0   ;  8  GEIGER
    .byte $FF,$02   ;  9  GLEEP
    .byte $04,$1C   ; 10  GLISSADE
    .byte $01,$10   ; 11  QWIP
    .byte $30,$0B   ; 12  OBOE
    .byte $30,$07   ; 13  FRENCH HORN
    .byte $50,$09   ; 14  ENGLISH HORN
    .byte $01,$64   ; 15  TIME BOMB

_play_sound_impl:
    cmp  #16            ; range check: clamp out-of-range indices to 0
    bcc  @lok
    lda  #0
@lok:
    asl  a              ; *2 — each table entry is 2 bytes
    tax
    lda  snd_table,x    ; load trip count
    sta  @trpcnt        ; self-modify (RAM, so writable at runtime)
    inx
    lda  snd_table,x    ; load sweep value
    sta  @sweep+1       ; self-modify LDY #imm operand below

    ; ---- outer sweep loop ----
    ; Y counts from sweep_value down to 1; each step = one pitch level.
    ; As Y decreases the WAIT delay shortens, so pitch sweeps upward.
@sweep:
    ldy  #0             ; operand overwritten above by sta @sweep+1

    ; ---- per-pitch inner loop ----
    ; X = number of speaker clicks at this pitch level (= Y initially).
    ; A = delay value (= Y); delay loop zeroes A, so TYA reloads it each cycle.
@nxtswp:
    tya                 ; A = current pitch delay
    tax                 ; X = click count for this pitch
@nxtcyc:
    tya                 ; A = pitch delay value
    sec                 ; SEC+SBC loop is the equivalent of the Monitor ROM WAIT
@delay:                 ; routine ($FCA8), inlined to avoid LC RAM / ROM bank issues
    sbc  #1
    bne  @delay
    bit  SPKR           ; read $C030 — side effect toggles speaker

    ; GEIGER mode: when sweep_value >= $80, the inner loop exits early
    ; at X = $80, producing a varying click density as Y sweeps down.
    cpx  #$80
    beq  @exit

    dex
    bne  @nxtcyc        ; repeat inner loop X times

    dey
    bne  @nxtswp        ; sweep to next (higher) pitch

    dec  @trpcnt        ; one full sweep done; more trips?
    bne  @sweep         ; yes — restart sweep from original pitch

@exit:
    rts

@trpcnt:
    .byte 1             ; mutable: overwritten at start of each call
