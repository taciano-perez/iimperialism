OVL_NAME_INDUSTRY:
    .byte   .strlen("ISCR")
    .byte   "ISCR"
OVL_NAME_PRODUCTION:
    .byte   .strlen("PSCR")
    .byte   "PSCR"
OVL_NAME_TRANSPORT:
    .byte   .strlen("TSCR")
    .byte   "TSCR"
OVL_NAME_ADMIRALTY:
    .byte   .strlen("ASCR")
    .byte   "ASCR"
OVL_NAME_DIPLOMACY:
    .byte   .strlen("DSCR")
    .byte   "DSCR"
OVL_NAME_TRADE_EXPEDITION_ACTION:
    .byte   .strlen("TXAC")
    .byte   "TXAC"
OVL_NAME_BATTLE:
    .byte   .strlen("BSCR")
    .byte   "BSCR"
OVL_NAME_SCIENCE:
    .byte   .strlen("SSCR")
    .byte   "SSCR"
OVL_NAME_GAME_MENU:
    .byte   .strlen("MENU")
    .byte   "MENU"
OVL_NAME_COUNCIL_NATIONS:
    .byte   .strlen("CNSL")
    .byte   "CNSL"

OVL_NAME_LO:
    .byte   <OVL_NAME_INDUSTRY
    .byte   <OVL_NAME_PRODUCTION
    .byte   <OVL_NAME_TRANSPORT
    .byte   <OVL_NAME_ADMIRALTY
    .byte   <OVL_NAME_DIPLOMACY
    .byte   <OVL_NAME_TRADE_EXPEDITION_ACTION
    .byte   <OVL_NAME_BATTLE
    .byte   <OVL_NAME_SCIENCE
    .byte   <OVL_NAME_GAME_MENU
    .byte   <OVL_NAME_COUNCIL_NATIONS

OVL_NAME_HI:
    .byte   >OVL_NAME_INDUSTRY
    .byte   >OVL_NAME_PRODUCTION
    .byte   >OVL_NAME_TRANSPORT
    .byte   >OVL_NAME_ADMIRALTY
    .byte   >OVL_NAME_DIPLOMACY
    .byte   >OVL_NAME_TRADE_EXPEDITION_ACTION
    .byte   >OVL_NAME_BATTLE
    .byte   >OVL_NAME_SCIENCE
    .byte   >OVL_NAME_GAME_MENU
    .byte   >OVL_NAME_COUNCIL_NATIONS
