    .include "checkers.inc"

    .segment "ZEROPAGE"

joy:    .tag Joy

    .segment "CODE"

    .proc Joy_update
    php
    seta16
    Joy_waitPoll
    lda joy_cur+0
    sta joy_prv+0
    lda joy_cur+2
    sta joy_prv+2
    lda JOY1L
    sta joy_cur+0
    lda JOY2L
    sta joy_cur+2
    lda #$0000
    plp
    rtl
    .endproc