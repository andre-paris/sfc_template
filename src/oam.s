    .include "checkers.inc"

    .segment "BSS"

oam:    .tag OAM

    .segment "CODE"

    .proc OAM_update
    jsl OAM_hideUnusedSprites
    jsl OAM_compactHiTable
    jml OAM_dma
    .endproc

    .proc OAM_compactHiTable
    php
    setai16
    ;------------------ Compact OAM High Table to 32 bytes
    ldx #0
    txy
    stx oam_idx
packloop:
    ; pack four sprites' size+xhi bits from oam_hi
    sep #$20
    lda oam_hi+13, y
    asl a
    asl a
    ora oam_hi+9, y
    asl a
    asl a
    ora oam_hi+5, y
    asl a
    asl a
    ora oam_hi+1, y
    sta oam_hi, x
    rep #$21  ; seta16 + clc for following addition
    ; move to the next set of 4 OAM entries
    inx
    tya
    adc #16
    tay
    cpx #32
    bcc packloop
    ;------------------
    plp
    rtl
    .endproc

    .proc OAM_hideUnusedSprites
    php
    setai16
    ldx oam_idx
hide:
    cpx #.sizeof(OAM::lo)
    bcs return
    lda #(225 << 8) | $ff   ; y=255 x=-1
    sta oam_x, x
    lda #$0100
    sta oam_hi, x
    .repeat 4
    inx
    .endrep
    jmp hide
return:
    plp
    rtl
    .endproc

    .proc OAM_dma
    php
    seta8
    seti16
    stz oam_dmaReady
    ldx #$0000
    stx OAMADDL
    stz DMAMODE
    lda #<OAMDATA
    sta DMADST
    ldx #.loword(oam_lo)
    stx DMASRC
    lda #.bankbyte(oam_lo)
    sta DMABNK
    ldx #544
    stx DMALEN
    lda #1
    ; sta MDMAEN
    sta oam_dmaReady
    plp
    rtl
    .endproc

    .func Obj_draw
    .palloc .zeropage, objX, 1
    .palloc .zeropage, objY, 1
    .palloc .zeropage, objHide, 1
    .palloc .zeropage, msp, 3
    php
    seta8
    seti16
    ;------------------ Set Data Bank
    lda msp+2
    phb
    pha
    plb
    ;------------------
    ldx oam_idx
    ldy #0
draw:
    lda (msp), y    ; sp x
    cmp #$80
    beq return
    clc
    adc objX
    sta oam_x, x
    iny
    lda (msp), y    ; sp y
    clc
    adc objY
    sta oam_y, x
    iny
    lda (msp), y    ; sp tile
    sta oam_tile, x
    iny
    lda (msp), y    ; sp attr
    ora #%00100000  ; set priority to 2
    sta oam_attr, x
    iny
    lda (msp), y    ; sp size
    ora objHide
    sta oam_hi+1, x
    iny
    .repeat 4
    inx
    .endrep
    jmp draw
return:
    stx oam_idx
    plb
    plp
    rts
    .endfunc