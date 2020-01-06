INCLUDE "hardware.inc"


SECTION "Header", ROM0[$100]


EntryPoint: ; This is where execution begins
    ;di ; Disable interrupts. That way we can avoid dealing with them, especially since we didn't talk about them yet :p
    nop
    jp Start ; Leave this tiny space

REPT $150 - $104
    db 0
ENDR

SECTION "Game code", ROM0[$150]
Start:
    di
    nop
    nop
    nop
