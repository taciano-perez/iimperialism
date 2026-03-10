    .export _ovl_trade_expedition_entry
    .import _render_trade_market

    .segment "CODE"

_ovl_trade_expedition_entry:
    jmp _render_trade_market
