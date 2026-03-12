    .export _ovl_trade_expedition_action_entry
    .import _handle_screen_trade_expedition

    .segment "CODE"

_ovl_trade_expedition_action_entry:
    jmp _handle_screen_trade_expedition
