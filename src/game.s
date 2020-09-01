    .include "sfc.inc"

    .export Game_main, Game_nmi, Game_irq

    .segment "CODE"

    .proc Game_main
    seta8
    
    ;------------------ Set backdrop color to blue
    stz SFC::CGADD
    lda #<RGB(0, 0, 31)
    sta SFC::CGDATA
    lda #>RGB(0, 0, 31)
    sta SFC::CGDATA
    ;------------------ Enable NMI+Joy read just for fun
    lda #NMI::ENABLE|NMI::JOY_READ
    sta SFC::NMITIMEN
    ;------------------ Turn on rendering
    lda #$0f
    sta SFC::INIDISP

forever:
    PPU_VSync
    jsl OAM_update
    jsl Joy_update
    jmp forever
    .endproc

    .proc Game_nmi
    seta16
    phb
    phk
    plb
    pha

    seta8
    lda oam_dmaReady
    beq ack_nmi
    sta SFC::MDMAEN
    stz oam_dmaReady

ack_nmi:
    bit SFC::RDNMI

    seta16
    pla
    plb
    rti
    .endproc

    .proc Game_irq
    rti
    .endproc