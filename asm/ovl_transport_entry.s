    .export _ovl_transport_entry
    .import _render_transport_screen

    .segment "CODE"

_ovl_transport_entry:
    jmp _render_transport_screen
