    ; Structs
    ; =======
    .struct Joy
    cur     .res 2*2
    prv     .res 2*2
    .endstruct

    joy_cur = joy+Joy::cur
    joy_prv = joy+Joy::prv

    ; Variables
    ; =========
    .globalzp joy

    ; Functions
    ; =========
    .global Joy_update

    ; Macros
    ; ======
    .macro Joy_waitPoll
    .local wait_pad
    lda #1
    wait_pad:
    bit HVBJOY
    bne wait_pad
    .endmac
