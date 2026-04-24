; Continuation code for the experimental qboot -> ProRWTS boot path.
;
; qboot loads this blob at $0400 and returns directly here. The continuation
; then runs ProRWTS init from the stage-2 blob at $0800 and loads the main game.

!cpu 6502
!to "rwts_continue",plain

*=$0400

; rwts_symbols_include

        prbyte         = $fdda
        prorwts_init   = $0800
        prorwts_open   = $bd00
        prodos_status  = $f3
        namlo          = $fb
        namhi          = $fc
        jsr     prorwts_init

        lda     #<game_name
        sta     namlo
        lda     #>game_name
        sta     namhi

        jsr     prorwts_open
        lda     prodos_status
        beq     launch_game

boot_hang
        jmp     boot_hang

launch_game
        lda     #<himem_entry
        sta     $73
        sta     $80
        lda     #>himem_entry
        sta     $74
        sta     $81

        lda     #$4c
        sta     startup_helper_probe_patch
        lda     #<startup_helper_probe_fallback
        sta     startup_helper_probe_patch+1
        lda     #>startup_helper_probe_fallback
        sta     startup_helper_probe_patch+2

        lda     #$20
        sta     startup_zerobss_call
        lda     #<zerobss_wrapper
        sta     startup_zerobss_call+1
        lda     #>zerobss_wrapper
        sta     startup_zerobss_call+2

        lda     #$4c
        sta     startup_callmain_jump
        lda     #<callmain_wrapper
        sta     startup_callmain_jump+1
        lda     #>callmain_wrapper
        sta     startup_callmain_jump+2

        jmp     game_entry

game_name
        !byte   4
        !text   "IIMP"

zerobss_wrapper
        jsr     zerobss_entry
        rts

callmain_wrapper
        jsr     main_entry
        jmp     boot_hang
