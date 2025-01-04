INCLUDE "hardware.inc"

SECTION "RST", ROM0[$0]

REPT $100 - $0
    db 0
ENDR

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
.notVBlanking
    ldh a, [rLY]
    cp 144
    jr c, .notVBlanking

    xor a
    ld [rLCDC], a
    call copyFont
    call copyString

    ld bc, $1234
    push bc
    ld bc, 0
    pop bc
    ld c, 1
    dec c
    dec c
    ld e, $FE
    inc e
    inc e

    ; Init display registers
    ld a, %11100100
    ld [rBGP], a

    xor a ; ld a, 0
    ld [rSCY], a
    ld [rSCX], a

    ; Shut sound down
    ld [rNR52], a

    ; Turn screen on, display background
    ld a, %10000001
    ld [rLCDC], a
    di

.end
    jr .end

copyFont:
    ld hl, $9000
    ld de, FontTiles
    ld bc, FontTilesEnd - FontTiles
.copyLoop:
    ld a, [de] ; Grab 1 byte from the source
    ld [hli], a ; Place it at the destination, incrementing hl
    inc de ; Move to next byte
    dec bc ; Decrement count
    ld a, b ; Check if count is 0, since `dec bc` doesn't update flags
    or c
    jr nz, .copyLoop
    ret
    nop

copyString:
    ld hl, $9800 ; This will print the string at the top-left corner of the screen
    ld de, HelloWorldStr
.copyStringLoop
    ld a, [de]
    ld [hli], a
    inc de
.printToSerial
    ld [$FF01], a ; Write the byte to the serial port
    and a ; Check if the byte we just copied is zero
    jr nz, .copyStringLoop ; Continue if it's not
    ret

SECTION "Font", ROM0

FontTiles:
INCBIN "font.chr"
FontTilesEnd:


SECTION "Strings", ROM0

HelloWorldStr:
    db "Hello World!", 0
