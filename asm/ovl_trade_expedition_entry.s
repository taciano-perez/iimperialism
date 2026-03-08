    .export _ovl_trade_expedition_entry
    .import _trade_expedition

    .segment "CODE"

_ovl_trade_expedition_entry:
    jmp _trade_expedition
