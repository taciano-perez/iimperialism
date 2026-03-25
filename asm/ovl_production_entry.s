    .export _ovl_production_entry
    .import _render_production_screen

    .segment "CODE"

_ovl_production_entry:
    jmp _render_production_screen
