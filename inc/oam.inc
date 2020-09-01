    ; Structs
    ; =======
    .struct Object
    xoff    .res 1
    yoff    .res 1
    tile    .res 1
    attr    .res 1
    .endstruct

    .struct OAM
    lo      .res .sizeof(Object)*128
    hi      .res .sizeof(Object)*128
    idx     .res 2
    dma     .res 1
    .endstruct

    oam_x =         oam+OAM::lo+Object::xoff
    oam_y =         oam+OAM::lo+Object::yoff
    oam_tile =      oam+OAM::lo+Object::tile
    oam_attr =      oam+OAM::lo+Object::attr
    oam_lo =        oam+OAM::lo
    oam_hi =        oam+OAM::hi
    oam_idx =       oam+OAM::idx
    oam_dmaReady =  oam+OAM::dma

    ; Variables
    ; =========
    .global oam

    ; Functions
    ; =========
    .global Obj_draw
    .global OAM_update
    .global OAM_compactHiTable
    .global OAM_hideUnusedSprites
    .global OAM_dma