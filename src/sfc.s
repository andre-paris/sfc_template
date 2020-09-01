;
; Definition of the internal header and vectors at $00FFC0-$00FFFF
; in the Super NES address space.
;

  .include "sfc.inc"

; LoROM is mapped to $808000-$FFFFFF in 32K blocks,
; skipping a15 and a23. Most is mirrored down to $008000.
MAPPER_LOROM = $20    ; a15 skipped
; HiROM is mapped to $C00000-$FFFFFF linearly.
; It is mirrored down to $400000, and the second half of
; each 64K bank is mirrored to $008000 and $808000.
MAPPER_HIROM = $21    ; C00000-FFFFFF skipping a22, a23
; ExHiROM is mapped to $C00000-$FFFFFF followed by
; $400000-$7FFFFF.  There are two inaccessible 32K holes
; near the end, and two 32K blocks that are accessible only
; through their mirrors to $3E8000 and $3F8000.
MAPPER_EXHIROM = $25  ; skipping a22, inverting a23

; If ROMSPEED_120NS is turned on, the game will be pressed
; on more expensive "fast ROM" that allows the CPU to run
; at its full 3.6 MHz when accessing ROM at $808000 and up.
ROMSPEED_200NS = $00
ROMSPEED_120NS = $10

; ROM and backup RAM sizes are expressed as
; ceil(log2(size in bytes)) - 10
MEMSIZE_NONE  = $00
MEMSIZE_2KB   = $01
MEMSIZE_4KB   = $02
MEMSIZE_8KB   = $03
MEMSIZE_16KB  = $04
MEMSIZE_32KB  = $05
MEMSIZE_64KB  = $06
MEMSIZE_128KB = $07  ; values from here up are for SRAM
MEMSIZE_256KB = $08  ; values from here down are for ROM
MEMSIZE_512KB = $09  ; Super Mario World
MEMSIZE_1MB   = $0A  ; Mario Paint
MEMSIZE_2MB   = $0B  ; Mega Man X, Super Mario All-Stars, original SF2
MEMSIZE_4MB   = $0C  ; Turbo/Super SF2, all Donkey Kong Country
MEMSIZE_8MB   = $0D  ; ExHiROM only: Tales of Phantasia, SF Alpha 2

REGION_JAPAN = $00
REGION_AMERICA = $01
REGION_PAL = $02

  .segment "SFCHEADER"
  .byte "  "          ; publisher ID, ASCII
  .byte "XLRW"        ; Game registration code (as in SNS-xxxx-USA)
  .res 6, $00         ; reserved
  .byte MEMSIZE_NONE  ; log2(backup flash size) - 10
  .byte MEMSIZE_NONE  ; log2(expansion work RAM size) - 10
  .byte 0             ; related to promo versions
  .byte 0             ; Coprocessor subtype

romname:
  ; The ROM name must be no longer than 21 characters.
  .byte "MCS Checkers"
  .assert * - romname <= 21, error, "ROM name too long"
  .if * - romname < 21
    .res romname + 21 - *, $20  ; space padding
  .endif
  .byte MAPPER_LOROM|ROMSPEED_120NS
  .byte $00   ; Cart type. 00: no extra RAM; 02: RAM with battery
  .byte MEMSIZE_256KB  ; log2(ROM size) - 10; 08-0C typical
  .byte MEMSIZE_NONE   ; log2(backup RAM size) - 10; 01,03,05 typical; Dezaemon has 07
  .byte REGION_AMERICA
  .byte $33   ; Publisher ID, or $33 for see 16 bytes before header
  .byte $00   ; ROM revision number
  .word $0000 ; sum of all bytes will be poked here after linking
  .word $0000 ; $FFFF minus above sum will also be poked here
  .res 4  ; unused vectors
  ; clcxce mode vectors
  ; reset unused because reset switches to 6502 mode
  .addr cop_handler, brk_handler, abort_handler
  .addr nmistub, $FFFF, irq_handler
  .res 4  ; more unused vectors
  ; 6502 mode vectors
  ; brk unused because 6502 mode uses irq handler and pushes the
  ; X flag clear for /IRQ or set for BRK
  .addr ecop_handler, $FFFF, eabort_handler
  .addr enmi_handler, resetstub, eirq_handler
  
  .segment "CODE0"

; Jumping out of bank $00 is especially important if you're using
; ROMSPEED_120NS.
nmistub:
  jml nmi_handler

irqstub:
  jml irq_handler

; Unused exception handlers
cop_handler:
brk_handler:
abort_handler:
ecop_handler:
eabort_handler:
enmi_handler:
eirq_handler:
  rti

  .import __ZEROPAGE_RUN__

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

  .proc resetstub
  sei                ; turn off IRQs
  clc
  xce                ; turn off 6502 emulation mode
  cld                ; turn off decimal ADC/SBC
  jml reset_fastrom  ; Bank $00 is not fast, but its mirror $80 is
  .endproc

  .segment "CODE"
  
  .proc reset_fastrom
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
  jml main
  .endproc

