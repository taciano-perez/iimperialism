; Continuation code for the experimental qboot -> ProRWTS boot path.
;
; qboot loads this blob at $0400 and returns directly here. Because $0400 is
; also text-screen memory on the Apple II, the bootstrap first relocates itself
; to $0300 before touching the display.

!cpu 6502
!to "rwts_continue",plain

*=$0400

; rwts_symbols_include

        relocate_dst                 = $0300
        text_mode_switch             = $c051
        full_screen_switch           = $c052
        page1_switch                 = $c054
        home_clear                   = $fc58
        prorwts_init                 = $0800
        prorwts_open                 = $bd00
        prodos_status                = $f3
        namlo                        = $fb
        namhi                        = $fc
        boot_hang_dst                = relocate_dst + (boot_hang - relocated_code)
        game_name_dst                = relocate_dst + (game_name - relocated_code)
        zerobss_wrapper_dst          = relocate_dst + (zerobss_wrapper - relocated_code)
        callmain_wrapper_dst         = relocate_dst + (callmain_wrapper - relocated_code)

entry:
        ldx     #$00
@copy:
        lda     relocated_code,x
        sta     relocate_dst,x
        inx
        cpx     #relocated_code_end-relocated_code
        bcc     @copy
        jmp     relocate_dst

relocated_code:
        bit     text_mode_switch
        bit     full_screen_switch
        bit     page1_switch
        jsr     home_clear

        jsr     prorwts_init

        lda     #<game_name_dst
        sta     namlo
        lda     #>game_name_dst
        sta     namhi

        jsr     prorwts_open
        lda     prodos_status
        beq     launch_game

boot_hang:
        jmp     boot_hang

launch_game:
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
        lda     #<zerobss_wrapper_dst
        sta     startup_zerobss_call+1
        lda     #>zerobss_wrapper_dst
        sta     startup_zerobss_call+2

        lda     #$4c
        sta     startup_callmain_jump
        lda     #<callmain_wrapper_dst
        sta     startup_callmain_jump+1
        lda     #>callmain_wrapper_dst
        sta     startup_callmain_jump+2

        jmp     game_entry

game_name:
        !byte   4
        !text   "IIMP"

zerobss_wrapper:
        jsr     zerobss_entry
        rts

callmain_wrapper:
        jsr     main_entry
        jmp     boot_hang_dst

relocated_code_end:
