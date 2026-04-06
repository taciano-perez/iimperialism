    .export _ovl_industry_entry
    .import _render_industry_screen

    .segment "CODE"

_ovl_industry_entry:
    jmp _render_industry_screen
