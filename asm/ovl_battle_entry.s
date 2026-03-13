    .export _ovl_battle_entry
    .import _render_battle_screen

    .segment "CODE"

_ovl_battle_entry:
    jmp _render_battle_screen
