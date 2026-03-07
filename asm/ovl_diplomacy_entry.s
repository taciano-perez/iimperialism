    .export _ovl_diplomacy_entry
    .import _render_diplomacy_screen

    .segment "CODE"

_ovl_diplomacy_entry:
    jmp _render_diplomacy_screen
