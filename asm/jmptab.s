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
