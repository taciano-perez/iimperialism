    .export _ovl_game_menu_entry
    .import _render_game_menu_screen

    .segment "CODE"

_ovl_game_menu_entry:
    jmp _render_game_menu_screen
