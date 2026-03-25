; Jump table for overlay resolution
; Segment JMPTAB lands immediately after STARTUP at $080F.
; Overlay binaries call these fixed addresses instead of the real functions.
;
;  $080F  JMP _clear_screen
;  $0812  JMP _clear_input_area
;  $0815  JMP _print
;  $0818  JMP _print_int_right_aligned
;  $081B  JMP _draw_picture_at
;  $081E  JMP _box
;  $0821  JMP _render_warehouse_box
;  $0824  JMP _cgetc
;  $0827  JMP _scan_uint
;  $082A  JMP _print_int
;  $082D  JMP _get_resource_name
;  $0830  JMP _get_relation_name
;  $0833  JMP _set_selected_trade_nation
;  $0836  JMP _get_selected_trade_nation
;  $0839  JMP _clear_area
;  $083C  JMP _paint_area
;  $083F  JMP _rand_range
;  $0842  JMP _start_new_game
;  $0845  JMP _print_int_right_aligned_currency
;  $0848  JMP _render_turn_funds_header
;  $084B  JMP _print_bold
;  $084E  JMP _wait_three_seconds_or_keypress
;  $0851  JMP _play_sound
;  $0854  JMP _play_sound_alert
;  $0857  JMP _cgetc_at
;  $085A  JMP _next_turn
;  $085D  JMP _run_overlay
;  $0860  JMP _production_orders

    .import _clear_screen
    .import _clear_input_area
    .import _print
    .import _print_int_right_aligned
    .import _draw_picture_at
    .import _box
    .import _render_warehouse_box
    .import _cgetc
    .import _scan_uint
    .import _print_int
    .import _get_resource_name
    .import _get_relation_name
    .import _set_selected_trade_nation
    .import _get_selected_trade_nation
    .import _clear_area
    .import _paint_area
    .import _rand_range
    .import _start_new_game
    .import _print_int_right_aligned_currency
    .import _render_turn_funds_header
    .import _print_bold
    .import _wait_three_seconds_or_keypress
    .import _play_sound
    .import _play_sound_alert
    .import _cgetc_at
    .import _next_turn
    .import _run_overlay
    .import _production_orders

    .segment "JMPTAB"

    jmp _clear_screen               ; $080F
    jmp _clear_input_area           ; $0812
    jmp _print                      ; $0815
    jmp _print_int_right_aligned    ; $0818
    jmp _draw_picture_at            ; $081B
    jmp _box                        ; $081E
    jmp _render_warehouse_box       ; $0821
    jmp _cgetc                      ; $0824
    jmp _scan_uint                  ; $0827
    jmp _print_int                  ; $082A
    jmp _get_resource_name          ; $082D
    jmp _get_relation_name          ; $0830
    jmp _set_selected_trade_nation  ; $0833
    jmp _get_selected_trade_nation  ; $0836
    jmp _clear_area                 ; $0839
    jmp _paint_area                 ; $083C
    jmp _rand_range                 ; $083F
    jmp _start_new_game             ; $0842
    jmp _print_int_right_aligned_currency ; $0845
    jmp _render_turn_funds_header   ; $0848
    jmp _print_bold                 ; $084B
    jmp _wait_three_seconds_or_keypress ; $084E
    jmp _play_sound                     ; $0851
    jmp _play_sound_alert               ; $0854
    jmp _cgetc_at                       ; $0857
    jmp _next_turn                      ; $085A
    jmp _run_overlay                    ; $085D
    jmp _production_orders              ; $0860
