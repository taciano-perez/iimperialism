    .export _ovl_science_entry
    .import _render_science_screen

    .segment "CODE"

_ovl_science_entry:
    jmp _render_science_screen
