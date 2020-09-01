  .include "sfc.inc"

  .import __ZEROPAGE_RUN__
  .import __MAPPER__
  .import __REGION__
  .import __ROMSIZE__
  .import __ROMSPEED__
  .import __WRAMSIZE__
  .import __SRAMSIZE__
  .import __SRAMINSIDE__
  .import Game_main, Game_nmi, Game_irq

  ; Game Header
  ; ===========

  .segment "SFCHEADER"
  .byte "  "          ; publisher ID, ASCII
  .byte "XLRW"        ; Game registration code (as in SNS-xxxx-USA)
  .res 6, $00         ; reserved
  .byte 0             ; log2(backup flash size) - 10
  .byte <__WRAMSIZE__  ; log2(expansion work RAM size) - 10
  .byte 0             ; related to promo versions
  .byte 0             ; Coprocessor subtype

romname:
  ; The ROM name must be no longer than 21 characters.
  .byte "SFC Template"
  .assert * - romname <= 21, error, "ROM name too long"
  .if * - romname < 21
    .res romname + 21 - *, $20  ; space padding
  .endif
  .byte <(__MAPPER__|__ROMSPEED__)
  .byte <__SRAMINSIDE__  ; Cart type. 00: no extra RAM; 02: RAM with battery
  .byte <__ROMSIZE__     ; log2(ROM size) - 10; 08-0C typical
  .byte <__SRAMSIZE__    ; log2(backup RAM size) - 10; 01,03,05 typical; Dezaemon has 07
  .byte <__REGION__      ; Game region
  .byte $33             ; Publisher ID, or $33 for see 16 bytes before header
  .byte $00             ; ROM revision number
  .word $0000           ; sum of all bytes will be poked here after linking
  .word $0000           ; $FFFF minus above sum will also be poked here
  .res 4  ; unused vectors
  ; clcxce mode vectors
  ; reset unused because reset switches to 6502 mode
  .addr cop_handler, brk_handler, abort_handler
  .addr nmi_handler, $FFFF, irq_handler
  .res 4  ; more unused vectors
  ; 6502 mode vectors
  ; brk unused because 6502 mode uses irq handler and pushes the
  ; X flag clear for /IRQ or set for BRK
  .addr ecop_handler, $FFFF, eabort_handler
  .addr enmi_handler, reset, eirq_handler
  
;--------------------------------------------------------------------------------------------------

  ; Interrupt Handlers
  ; ==================

  .segment "CODE0"

; Jumping out of bank $00 is especially important if you're using ROMSPEED_120NS.
nmi_handler:
  jml Game_nmi

irq_handler:
  jml Game_irq

; Unused exception handlers
cop_handler:
brk_handler:
abort_handler:
ecop_handler:
eabort_handler:
enmi_handler:
eirq_handler:
  rti

;--------------------------------------------------------------------------------------------------

  ; System Reset
  ; ============

; Mask off low byte to allow use of $000000-$00000F as local variables
ZEROPAGE_BASE   = __ZEROPAGE_RUN__ & $FF00

; Make sure these conform to the linker script (e.g. lorom256.cfg).
STACK_BASE      = $0100
STACK_SIZE      = $0100
LAST_STACK_ADDR = STACK_BASE + STACK_SIZE - 1

PPU_BASE        = $2100
CPUIO_BASE      = $4200

; MMIO is mirrored into $21xx, $42xx, and $43xx of all banks $00-$3F
; and $80-$BF.  To make it work no matter the current data bank, we
; can use a long address in a nonzero bank.
; Bit 0 of MEMSEL24 enables fast ROM access above $808000.
MEMSEL24          = $80420D

; Bit 4 of the byte at $FFD5 in the cartridge header specifies
; whether a game should be manufactured with slow or fast ROM.
; The init code enables fast ROM if this bit is true.
map_mode        = $00FFD5

; A tiny stub in bank $00 needs to set interrupt priority to 1,
; leave 6502 emulation mode, and long jump to the rest of init code
; in another bank. This should set 16-bit mode, turn off decimal
; mode, set the stack pointer, load a predictable state into writable
; MMIO ports of the S-PPU and S-CPU, and set the direct page base.
; For explanation of the values that this writes, see docs/init.txt
;
; For advanced users: Long stretches of STZ are a useful place to
; shuffle code when watermarking your binary.

  .proc reset
  sei                ; turn off IRQs
  clc
  xce                ; turn off 6502 emulation mode
  cld                ; turn off decimal ADC/SBC
  jml fastReset
  .endproc

  .segment "CODE"
  
  .proc fastReset
  rep #$30         ; 16-bit AXY
  ldx #LAST_STACK_ADDR
  txs              ; set the stack pointer

  ; Initialize the CPU I/O registers to predictable values
  lda #CPUIO_BASE
  tad              ; temporarily move direct page to S-CPU I/O area
  lda #$FF00
  sta $00     ; disable NMI and HVIRQ; don't drive controller port pin 6
  stz $02     ; clear multiplier factors
  stz $04     ; clear dividend
  stz $06     ; clear divisor and low byte of hcount
  stz $08     ; clear high bit of hcount and low byte of vcount
  stz $0A     ; clear high bit of vcount and disable DMA copy
  stz $0C     ; disable HDMA and fast ROM

  ; Initialize the PPU registers to predictable values
  lda #PPU_BASE
  tad              ; temporarily move direct page to PPU I/O area

  ; first clear the regs that take a 16-bit write
  lda #$0080
  sta $00     ; Forced blank, brightness 0, sprite size 8/16 from VRAM $0000
  stz $02     ; OAM address = 0
  stz $05     ; BG mode 0, no mosaic
  stz $07     ; BG 1-2 map 32x32 from VRAM $0000
  stz $09     ; BG 3-4 map 32x32 from VRAM $0000
  stz $0B     ; BG tiles from $0000
  stz $16     ; VRAM address $0000
  stz $23     ; disable BG window
  stz $26     ; clear window 1 x range
  stz $28     ; clear window 2 x range
  stz $2A     ; clear window mask logic
  stz $2C     ; disable all layers on main and sub
  stz $2E     ; disable all layers on main and sub in window
  ldx #$0030
  stx $30     ; disable color math and mode 3/4/7 direct color
  ldy #$00E0
  sty $32     ; clear RGB components of COLDATA; disable interlace+pseudo hires

  ; now clear the regs that need 8-bit writes
  sep #$20
  sta $15     ; still $80: add 1 to VRAM pointer after high byte write
  stz $1A     ; enable mode 7 wrapping and disable flipping
  stz $21     ; set CGRAM address to color 0
  stz $25     ; disable obj and math window

  ; The scroll registers $210D-$2114 need double 8-bit writes
  .repeat 8, I
    stz $0D+I
    stz $0D+I
  .endrepeat

  ; As do the mode 7 registers, which we set to the identity matrix
  ; [ $0100  $0000 ]
  ; [ $0000  $0100 ]
  lda #$01
  stz $1B
  sta $1B
  stz $1C
  stz $1C
  stz $1D
  stz $1D
  stz $1E
  sta $1E
  stz $1F
  stz $1F
  stz $20
  stz $20

  ; Set fast ROM if the internal header so requests
  lda f:map_mode
  and #$10
  beq not_fastrom
  inc a
not_fastrom:
  sta MEMSEL24

  rep #$20
  lda #ZEROPAGE_BASE
  tad                 ; return direct page to real zero page

  ; Unlike on the NES, we don't have to wait 2 vblanks to do
  ; any of the following remaining tasks.
  ; * Fill or clear areas of VRAM that will be used
  ; * Clear areas of WRAM that will be used
  ; * Load palette data into CGRAM
  ; * Fill shadow OAM and then copy it to OAM
  ; * Boot the S-SMP
  ; The main routine can do these in any order.
  jml Game_main
  .endproc

;--------------------------------------------------------------------------------------------------

  ; OAM functions
  ; =============

  .segment "BSS"

oam:  .tag OAM

  .segment "CODE"

  .proc OAM_update
  jsl OAM_hideUnusedSprites
  jsl OAM_compactHiTable
  jml OAM_doDMA
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

  .proc OAM_doDMA
  php
  seta8
  seti16
  stz oam_dmaReady
  ldx #$0000
  stx SFC::OAMADDL
  stz DMA::MODE0
  lda #<SFC::OAMDATA
  sta DMA::DST0
  ldx #.loword(oam_lo)
  stx DMA::SRC0
  lda #.bankbyte(oam_lo)
  sta DMA::BNK0
  ldx #544
  stx DMA::LEN0
  lda #1
  sta oam_dmaReady
  plp
  rtl
  .endproc

;--------------------------------------------------------------------------------------------------

  ; Joypad functions
  ; ================

  .segment "ZEROPAGE"

joy:    .tag Joy

  .segment "CODE"

  .proc Joy_update
  php
  seta16
  Joy_WaitPoll
  lda joy_curState+0
  sta joy_prvState+0
  lda joy_curState+2
  sta joy_prvState+2
  lda SFC::JOY1L
  sta joy_curState+0
  lda SFC::JOY2L
  sta joy_curState+2
  plp
  rtl
  .endproc

;--------------------------------------------------------------------------------------------------
