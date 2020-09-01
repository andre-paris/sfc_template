  .p816
  .smart

  ; Super Famicom Registers
  ; =======================
  ;
  ; Source: https://wiki.superfamicom.org/registers
  ;
  ; Register Explanations
  ; =====================
  ; The flags are:
  ;
  ; rw?fvha
  ; ||||||+--> '+' if it can be read/written at any time, '-' otherwise
  ; |||||+---> '+' if it can be read/written during H-Blank
  ; ||||+----> '+' if it can be read/written during V-Blank
  ; |||+-----> '+' if it can be read/written during force-blank
  ; ||+------> Read/Write style: 'b'     => byte
  ; ||                           'h'/'l' => read/write high/low byte of a word
  ; ||                           'w'     => word read/write twice low then high
  ; |+-------> 'w' if the register is writable for an effect
  ; +--------> 'r' if the register is readable for a value or effect (i.e. not [open bus](/open-bus)).
  .enum SFC
  INIDISP = $2100	; Screen Display
  ; $2100  wb++++
  ;          x---bbbb
  ;          x        = Force blank on when set.
  ;              bbbb = Screen brightness, F=max, 0="off".
  ; Note that force blank CAN be disabled mid-scanline. However, this can result in glitched graphics 
  ; on that scanline, as the internal rendering buffers will not have been updated during force blank. 
  ; Current theory is that BGs will be glitched for a few tiles (depending on how far in advance the 
  ; PPU operates), and OBJ will be glitched for the entire scanline. Also, writing this register on 
  ; the first line of V-Blank (225 or 240, depending on overscan) when force blank is currently active 
  ; causes the OAM Address Reset to occur.

  OBSEL	; Object Size and Character Address
  ; $2101  wb++?- 
  ;          sssnnbbb
  ;          sss       = Object size:
  ;                000 =  8x8  and 16x16 sprites
  ;                001 =  8x8  and 32x32 sprites
  ;                010 =  8x8  and 64x64 sprites
  ;                011 = 16x16 and 32x32 sprites
  ;                100 = 16x16 and 64x64 sprites
  ;                101 = 32x32 and 64x64 sprites
  ;                110 = 16x32 and 32x64 sprites ('undocumented')
  ;                111 = 16x32 and 32x32 sprites ('undocumented')
  ;             nn     = Name Select
  ;               bbb  = Name Base Select (Addr>>14)
  ; 
  ; @see https://wiki.superfamicom.org/sprites

  OAMADDL	; OAM Address low byte
  OAMADDH	; OAM Address high bit and Obj Priority
  ; 2102  wl++?-
  ; 2103  wh++?-
  ;         p------b aaaaaaaa
  ;         p                 = Obj Priority activation bit
  ;                b aaaaaaaa = OAM address
  ; When Obj Priority activation bit is set, an Obj other than Sprite 0 may be given priority. 
  ; @see https://wiki.superfamicom.org/sprites

  ; OAM address can be thought of in two ways, depending on your conception of OAM. If you consider 
  ; OAM as a 544-byte table, baaaaaaaa is the word address into that table. If you consider OAM to be 
  ; a 512-byte table and a 32-byte table, b is the table selector and aaaaaaaa is the word address in 
  ; the table. 
  ; @see https://wiki.superfamicom.org/sprites

  ; The internal OAM address is invalidated when scanlines are being rendered. This invalidation is 
  ; deterministic, but we do not know how it is determined. Thus, the last value written to these 
  ; registers is reloaded into the internal OAM address at the beginning of V-Blank if that occurs 
  ; outside of a force-blank period. This is known as 'OAM reset'. 'OAM reset' also occurs on certain 
  ; writes to $2100.

  ; Writing to either $2102 or $2103 resets the entire internal OAM Address to the values last written 
  ; to this register. E.g., if you set $104 to this register, write 4 bytes, then write $1 to $2103, 
  ; the internal OAM address will point to word 4, not word 6.

  OAMDATA	; Data for OAM write
  ; 2104  wb++--
  ;         dddddddd
  ; Note that OAM writes are done in an odd manner, in particular the low table of OAM is not affected 
  ; until the high byte of a word is written (however, the high table is affected immediately). Thus, 
  ; if you set the address, then alternate writes and reads, OAM will never be affected until you 
  ; reach the high table!

  ; Similarly, if you set the address to 0, then write 1, 2, read, then write 3, OAM will end up as 
  ; "01 02 01 03", rather than "01 02 xx 03" as you might expect.

  ; Technically, this register CAN be written during H-blank (and probably mid-scanline as well). 
  ; However, due to OAM address invalidation the actual OAM byte written will probably not be what you 
  ; expect. Note that writing during force-blank will only work as expected if that force-blank was 
  ; begun during V-Blank, or (probably) if $2102/3 have been reset during that force-blank period.

  ; 
  ; @see https://wiki.superfamicom.org/sprites

  BGMODE	; BG Mode and Character Size
  ; 2105  wb+++-
  ;         DCBAemmm

  ;         A/B/C/D   = BG character size for BG1/BG2/BG3/BG4
  ;              mmm  = BG Mode
  ;             e     = Mode 1 BG3 priority bit
  ;             Mode     BG depth  OPT  Priorities
  ;                      1 2 3 4        Front -> Back
  ;             -=-------=-=-=-=----=---============---
  ;              0       2 2 2 2    n    3AB2ab1CD0cd
  ;              1       4 4 2      n    3AB2ab1C 0c
  ;                         * if e set: C3AB2ab1  0c
  ;              2       4 4        y    3A 2B 1a 0b
  ;              3       8 4        n    3A 2B 1a 0b
  ;              4       8 2        y    3A 2B 1a 0b
  ;              5       4 2        n    3A 2B 1a 0b
  ;              6       4          y    3A 2  1a 0
  ;              7       8          n    3  2  1a 0
  ;              7+EXTBG 8 7        n    3  2B 1a 0b
  ; If the BG character size for BG1/BG2/BG3/BG4 bit is set, then the BG is made of 16x16 tiles. 
  ; Otherwise, 8x8 tiles are used. However, note that Modes 5 and 6 always use 16-pixel wide tiles, 
  ; and Mode 7 always uses 8x8 tiles. 
  ; @see https://wiki.superfamicom.org/backgrounds

  ; "OPT" means "Offset-per-tile mode". For the priorities, numbers mean sprites with that priority. 
  ; Letters correspond to BGs (A=1, B=2, etc), with upper/lower case indicating tile priority 1/0. 
  ; @see https://wiki.superfamicom.org/backgrounds

  ; Mode 7's EXTBG mode allows you to enable BG2, which uses the same tilemap and character data as 
  ; BG1 but interprets bit 7 of the pixel data as a priority bit. BG2 also has some oddness to do with 
  ; some of the per-BG registers below. See the Mode 7 section under BACKGROUNDS for details.

  MOSAIC	; Screen Pixelation
  ; 2106  wb+++-
  ;         xxxxDCBA
  ;             A/B/C/D = Affect BG1/BG2/BG3/BG4
  ;         xxxx        = pixel size, 0=1x1, F=16x16
  ; The mosaic filter goes over the BG and covers each x-by-x square with the upper-left pixel of that 
  ; square, with the top of the first row of squares on the 'starting scanline'. If this register is 
  ; set during the frame, the 'starting scanline' is the current scanline, otherwise it is the first 
  ; visible scanline of the frame. I.e. if even scanlines are completely red and odd scanlines are 
  ; completely blue, setting the xxxx=1 mid-frame will make the rest of the screen either completely 
  ; red or completely blue depending on whether you set xxxx on an even or an odd scanline.

  ; XXX: It seems that writing the same value to this register does not reset the 'starting scanline', 
  ; but which changes do reset it?

  ; Note that mosaic is applied after scrolling, but before any clip windows, color windows, or math. 
  ; So the XxX block can be partially clipped, and it can be mathed as normal with a non-mosaiced BG. 
  ; But scrolling can't make it partially one color and partially another.

  ; Modes 5-6 should 'double' the expansion factor to expand half-pixels. This actually makes xxxx=0 
  ; have a visible effect, since the even half-pixels (usually on the subscreen) hide the odd half-
  ; pixels. The same thing happens vertically with interlace mode.

  ; Mode 7, of course, is weird. BG1 mosaics about like normal, as long as you remember that the Mode 
  ; 7 transformations have no effect on the XxX blocks. BG2 uses bit A to control 'vertical mosaic' 
  ; and bit B to control 'horizontal mosaic', so you could be expanding over 1xX, Xx1, or XxX blocks. 
  ; This can get really interesting as BG1 still uses bit A as normal, so you could have the BG1 
  ; pixels expanded XxX with high-priority BG2 pixels expanded 1xX on top of them.

  ; 
  ; @see https://wiki.superfamicom.org/backgrounds

  BG1SC	; BG1 Tilemap Address and Size
  BG2SC	; BG2 Tilemap Address and Size
  BG3SC	; BG3 Tilemap Address and Size
  BG4SC	; BG4 Tilemap Address and Size
  ; 2107  wb++?-
  ; 2108  wb++?-
  ; 2109  wb++?-
  ; 210a  wb++?-
  ;         aaaaaayx
  ;         aaaaaa      = Tilemap address in VRAM (Addr>>10)
  ;                x    = Tilemap horizontal mirroring
  ;               y     = Tilemap veritcal mirroring
  ; All tilemaps are 32x32 tiles. If x and y are both unset, there is one tilemap at Addr. If x is 
  ; set, a second tilemap follows the first that should be considered "to the right of" the first. If 
  ; y is set, a second tilemap follows the first that should be considered "below" the first. If both 
  ; are set, then a second follows "to the right", then a third "below", and a fourth "below and to 
  ; the right".

  ; 
  ; @see https://wiki.superfamicom.org/backgrounds

  BG12NBA	; BG1 and 2 Chr Address
  BG34NBA	; BG3 and 4 Chr Address
  ; 210b  wb++?-
  ; 210c  wb++?-
  ;         bbbbaaaa
  ;             aaaa = Base address for BG1/3 (Addr>>12)
  ;         bbbb     = Base address for BG2/4 (Addr>>12)
  ; Simply spoken: Saving "$63" into $210B makes the PPU look for the Tileset for BG2 at $6000 in the 
  ; VRAM and for BG1 at $3000.

  ; 
  ; @see https://wiki.superfamicom.org/backgrounds

  BG1HOFS	; BG1 Horizontal Scroll
  BG1VOFS	; BG1 Vertical Scroll
  ; 210d  ww+++-
  ;       ww+++-
  ; 210e  ww+++-
  ;       ww+++-
  ;         ------xx xxxxxxxx
  ;         ---mmmmm mmmmmmmm

  ;               x = The BG offset, 10 bits.
  ;            m    = The Mode 7 BG offset, 13 bits two's-complement signed.
  ; These are actually two registers in one (or would that be "4 registers in 2"?). Anyway, writing 
  ; $210d will write both BG1HOFS which works exactly like the rest of the BGnxOFS registers below 
  ; ($210f-$2114), and M7HOFS which works with the M7* registers ($211b-$2120) instead.

  ; Modes 0-6 use BG1xOFS and ignore M7xOFS, while Mode 7 uses M7xOFS and ignores BG1HOFS. See the 
  ; appropriate sections below for details, and note the different formulas for BG1HOFS versus M7HOFS.

  BG2HOFS	; BG2 Horizontal Scroll
  BG2VOFS	; BG2 Vertical Scroll
  BG3HOFS	; BG3 Horizontal Scroll
  BG3VOFS	; BG3 Vertical Scroll
  BG4HOFS	; BG4 Horizontal Scroll
  BG4VOFS	; BG4 Vertical Scroll
  ; 210f  ww+++-
  ; 2110  ww+++-
  ; 2111  ww+++-
  ; 2112  ww+++-
  ; 2113  ww+++-
  ; 2114  ww+++-
  ;         ------xx xxxxxxxx
  ; Note that these are "write twice" registers, first the low byte is written then the high. Current 
  ; theory is that writes to the register work like this:

  ; BGnHOFS = (Current<<8) | (Prev1&~7) | (Prev2&7);
  ; Prev1 = Current;
  ; Prev2 = Current;
  ; or
  ; BGnVOFS = (Current<<8) | Prev1;
  ; Prev1 = Current;
  ; Note that there is only one Prev1 shared by all eight BGnxOFS registers, and only one Prev2 shared 
  ; by the four BGnHOFS registers. These are NOT shared with the M7* registers (not even M7xOFS and 
  ; BG1xOFS).

  ; x = The BG offset, at most 10 bits (some modes effectively use as few as 8).
  ; Note that all BGs wrap if you try to go past their edges. Thus, the maximum offset value in BG 
  ; Modes 0-6 is 1023, since you have at most 64 tiles (if x/y of BGnSC is set) of 16 pixels each (if 
  ; the appropriate bit of BGMODE is set).

  ; Horizontal scrolling scrolls in units of full pixels no matter if we're rendering a 256-pixel wide 
  ; screen or a 512-half-pixel wide screen. However, vertical scrolling will move in half-line 
  ; increments if interlace mode is active.

  ; 
  ; @see https://wiki.superfamicom.org/backgrounds

  VMAIN	; Video Port Control
  ; 2115  wb++?-
  ;         i---mmii
  ;         i          = Address increment mode^:
  ;                      0 => increment after writing &#36;2118/reading &#36;2139
  ;                      1 => increment after writing &#36;2119/reading &#36;213a
  ;               ii   = Address increment amount
  ;                      00 = Normal increment by 1
  ;                      01 = Increment by 32
  ;                      10 = Increment by 128
  ;                      11 = Increment by 128
  ;             mm     = Address remapping
  ;                      00 = No remapping
  ;                      01 = Remap addressing aaaaaaaaBBBccccc => aaaaaaaacccccBBB
  ;                      10 = Remap addressing aaaaaaaBBBcccccc => aaaaaaaccccccBBB
  ;                      11 = Remap addressing aaaaaaBBBccccccc => aaaaaacccccccBBB
  ; ^:Note that a word write stores low first, then high. Thus, if you're storing a word value to 
  ; $2118/9, you'll probably want to set 1 here.

  ; The "remap" modes basically implement address translation. If $2116/7 are set to #$0003, then word 
  ; address #$0018 will be written instead, and $2116/7 will be incremented to $0004.

  VMADDL	; VRAM Address low byte
  VMADDH	; VRAM Address high byte
  ; 2116  wl++?-
  ; 2117  wh++?-
  ;         aaaaaaaa aaaaaaaa
  ; This sets the address for $2118/9 and $2139/a. Note that this is a word address, not a byte 
  ; address! 
  ; @see https://wiki.superfamicom.org/backgrounds
  ; @see https://wiki.superfamicom.org/sprites

  VMDATAL	; VRAM Data Write low byte
  VMDATAH	; VRAM Data Write high byte
  ; 2118  wl++--
  ; 2119  wh++--
  ;         xxxxxxxx xxxxxxxx
  ; This writes data to VRAM. The writes take effect immediately(?), even if no increment is 
  ; performed. The address is incremented when one of the two bytes is written; which one depends on 
  ; the setting of bit 7 of register $2115. Keep in mind the address translation bits of $2115 as 
  ; well. The interaction between these registers and $2139/a is unknown. See the sections 
  ; "BACKGROUNDS" and "SPRITES" below for details.

  M7SEL	; Mode 7 Settings
  ; 211a  wb++?-
  ;         rc----yx
  ;         r        = Playing field size^
  ;          c       = Empty space fill, when bit 7 is set:
  ;                    0 = Transparent.
  ;                    1 = Fill with character 0. Note that the fill is matrix transformed like all 
  ; other Mode 7 tiles.
  ;              x/y = Horizontal/Veritcal mirroring. If the bit is set, flip the 256x256 pixel 
  ; 'screen' in that direction.
  ; ^When clear, the playing field is 1024x1024 pixels (so the tilemap completely fills it). When set, 
  ; the playing field is much larger, and the 'empty space' fill is controlled by bit 6. 
  ; @see https://wiki.superfamicom.org/backgrounds

  M7A	; Mode 7 Matrix A (also used with $2134/6)
  M7B	; Mode 7 Matrix B (also used with $2134/6)
  M7C	; Mode 7 Matrix C
  M7D	; Mode 7 Matrix D
  ; 211b  ww+++-
  ; 211c  ww+++-
  ; 211d  ww+++-
  ; 211e  ww+++-
  ;         aaaaaaaa aaaaaaaa
  ; Note that these are "write twice" registers, first the low byte is written then the high. Current 
  ; theory is that writes to the register work like this:

  ; Reg = (Current<<8) | Prev;
  ; Prev = Current;
  ; Note that there is only one Prev shared by all these registers. This Prev is NOT shared with the 
  ; BGnxOFS registers, but it IS shared with the M7xOFS registers. These set the matrix parameters for 
  ; Mode 7. The values are an 8-bit fixed point, i.e. the value should be divided by 256.0 when used 
  ; in calculations. See below for more explanation. The product A*(B>>8) may be read from registers 
  ; $2134/6. There is supposedly no important delay. It may not be operative during Mode 7 rendering. 
  ; @see https://wiki.superfamicom.org/backgrounds

  M7X	; Mode 7 Center X
  M7Y	; Mode 7 Center Y
  ; 211f  ww+++-
  ; 2120  ww+++-
  ;         ---xxxxx xxxxxxxx
  ; Note that these are "write twice" registers, like the other M7* registers. See above for the write 
  ; semantics. The value is 13 bit two's-complement signed. The matrix transformation formula is:

  ; [ X ]   [ A B ]   [ SX + M7HOFS - CX ]   [ CX ]
  ; [   ] = [     ] * [                  ] + [    ]
  ; [ Y ]   [ C D ]   [ SY + M7VOFS - CY ]   [ CY ]
  ; Note: SX/SY are screen coordinates. X/Y are coordinates in the playing field from which the pixel 
  ; is taken. If $211a bit 7 is clear, the result is then restricted to 0<=X<=1023 and 0<=Y<=1023. If 
  ; $211a bits 6 and 7 are both set and X or Y is less than 0 or greater than 1023, use the low 3 bits 
  ; of each to choose the pixel from character 0. The bit-accurate formula seems to be something along 
  ; the lines of:

  ;   #define CLIP(a) (((a)&0x2000)?((a)|~0x3ff):((a)&0x3ff))

  ;   X[0,y] = ((A*CLIP(HOFS-CX))&~63)
  ;          + ((B*y)&~63) + ((B*CLIP(VOFS-CY))&~63)
  ;          + (CX<<8)
  ;   Y[0,y] = ((C*CLIP(HOFS-CX))&~63)
  ;          + ((D*y)&~63) + ((D*CLIP(VOFS-CY))&~63)
  ;          + (CY<<8)

  ;   X[x,y] = X[x-1,y] + A
  ;   Y[x,y] = Y[x-1,y] + C

  ; (In all cases, X[] and Y[] are fixed point with 8 bits of fraction)
  ; 
  ; @see https://wiki.superfamicom.org/backgrounds

  CGADD	; CGRAM Address
  ; 2121  wb+++-
  ;         cccccccc
  ; This sets the word address (i.e. color) which will be affected by $2122 and $213b.

  ; Writing "0" to $2121 will change the "currently selected color index" used by $2122, to 0. Upon 
  ; writing a color to $2122, the color will be stored into the array index selected by $2121, which 
  ; in this case would be 0 - if you wrote 0 to $2121 before writing a color to $2122.

  ; Keep in mind the color index accessed by $2121 will automatically increment by 1 after writing a 
  ; color to $2122. This is an effect generated by $2122 after being used in case you want to write 
  ; specific colors in a series.

  CGDATA	; CGRAM Data write
  ; 2122  ww+++-
  ;         -bbbbbgg gggrrrrr
  ; This writes to CGRAM, effectively setting the palette colors. Accesses to CGRAM are handled just 
  ; like accesses to the low table of OAM, see $2104 for details. Note that the color values are 
  ; stored in BGR order.

  W12SEL	; Window Mask Settings for BG1 and BG2
  W34SEL	; Window Mask Settings for BG3 and BG4
  WOBJSEL	; Window Mask Settings for OBJ and Color Window
  ; 2123  wb+++-
  ; 2124  wb+++-
  ; 2125  wb+++-
  ;         ABCDabcd
  ;                d = Window 1 Inversion for BG1/BG3/OBJ
  ;               c  = Enable window 1 for BG1/BG3/OBJ
  ;              b   = Window 2 Inversion for BG1/BG3/OBJ
  ;             a    = Enable window 2 for BG1/BG3/OBJ
  ;            D     = Window 1 Inversion for BG2/BG4/Color^^
  ;           C      = Enable window 1 for BG2/BG4/Color^
  ;          B       = Window 2 Inversion for BG2/BG4/Color^^
  ;         A        = Enable window 2 for BG2/BG4/Color^
  ; ^When the bit is set, the corresponding window will affect the corresponding background (subject 
  ; to the settings of $212e/f).

  ; ^^When the bit is set, "W" should be replaced by "~W" (not-W) in the window combination formulae 
  ; below. 
  ; @see https://wiki.superfamicom.org/windows

  WH0	; Window 1 Left Position
  WH1	; Window 1 Right Position
  WH2	; Window 2 Left Position
  WH3	; Window 2 Right Position
  ; 2126  wb+++-
  ; 2127  wb+++-
  ; 2128  wb+++-
  ; 2129  wb+++-
  ;         xxxxxxxx
  ; These set the offset of the appropriate edge of the appropriate window. Note that if the left edge 
  ; is greater than the right edge, the window is considered to have no range at all (and thus "W" 
  ; always is false). 
  ; @see https://wiki.superfamicom.org/windows

  WBGLOG	; Window mask logic for BGs
  WOBJLOG	; Window mask logic for OBJs and Color Window
  ; 212a  wb+++- 
  ;         44332211
  ; 212b  wb+++- 
  ;         ----ccoo

  ;         44/33/22/11/oo/cc = Mask logic for BG1/BG2/BG3/BG4/OBJ/Color
  ;             This specified the window combination method, using standard boolean operators:
  ;               00 = OR
  ;               01 = AND
  ;               10 = XOR
  ;               11 = XNOR
  ; Consider two variables, W1 and W2, which are true for pixels between the appropriate left and 
  ; right bounds as set in $2126-$2129 and false otherwise. Then, you have the following 
  ; possibilities: (replace "W#" with "~W#", depending on the Inversion settings of $2123-$2125) 
  ; Neither window enabled => nothing masked. One window enabled => Either W1 or W2, as appropriate. 
  ; Both windows enabled => W1 op W2, where "op" is as above. Where the function is true, the BG will 
  ; be masked. 
  ; @see https://wiki.superfamicom.org/windows

  TM	; Main Screen Designation
  TS	; Subscreen Designation
  ; 212c  wb+++-
  ; 212d  wb+++-
  ;         ---o4321

  ;         1/2/3/4/o = Enable BG1/BG2/BG3/BG4/OBJ for display on the main (or sub) screen.
  ; 
  ; @see https://wiki.superfamicom.org/backgrounds

  TMW	; Window Mask Designation for the Main Screen
  TSW	; Window Mask Designation for the Subscreen
  ; 212e  wb+++-
  ; 212f  wb+++-
  ;         ---o4321
  ;         1/2/3/4/o = Enable window masking for BG1/BG2/BG3/BG4/OBJ on the main (or sub) screen.
  ; 
  ; @see https://wiki.superfamicom.org/backgrounds

  CGWSEL	; Color Addition Select
  ; 2130  wb+++-
  ;         ccmm--sd
  ;         cc       = Clip colors to black before math
  ;                    00 => Never
  ;                    01 => Outside Color Window only
  ;                    10 => Inside Color Window only
  ;                    11 => Always
  ;           mm     = Prevent color math
  ;                    00 => Never
  ;                    01 => Outside Color Window only
  ;                    10 => Inside Color Window only
  ;                    11 => Always
  ;               s  = Add subscreen (instead of fixed color)
  ;                d = Direct color mode for 256-color BGs
  ; @see https://wiki.superfamicom.org/backgrounds
  ; @see https://wiki.superfamicom.org/windows
  ; @see https://wiki.superfamicom.org/redering-the-screen

  CGADSUB	; Color math designation
  ; 2131  wb+++-
  ;         shbo4321
  ;         s             = Add/subtract select
  ;                         0 => Add the colors
  ;                         1 => Subtract the colors
  ;          h            = Half color math.^
  ;           4/3/2/1/o/b = Enable color math on BG1/BG2/BG3/BG4/OBJ/Backdrop
  ; ^ When set, the result of the color math is divided by 2 (except when $2130 bit 1 is set and the 
  ; fixed color is used, or when color is clipped). 
  ; @see https://wiki.superfamicom.org/backgrounds
  ; @see https://wiki.superfamicom.org/windows
  ; @see https://wiki.superfamicom.org/rendering-the-screen

  COLDATA	; Fixed Color Data
  ; 2132  wb+++-
  ;         bgrccccc
  ;         b/g/r    = Which color plane(s) to set the intensity for.
  ;            ccccc = Color intensity.
  ; So basically, to set an orange you'd do something along the lines of: LDA #$3f STA $2132 LDA #$4f 
  ; STA $2132 LDA #$80 STA $2132

  ; @see https://wiki.superfamicom.org/backgrounds
  ; @see https://wiki.superfamicom.org/windows

  SETINI	; Screen Mode/Video Select
  ; 2133  wb+++-
  ;         se--poIi
  ;         s        = "External Sync".^
  ;          e       = Mode 7 EXTBG ("Extra BG").^^
  ;             p    = Enable pseudo-hires mode.^^^
  ;              o   = Overscan mode.^^^^
  ;               I  = OBJ Interlace.^^^^^
  ;                i = Screen interlace.^^^^^^
  ; ^Used for superimposing "sfx" graphics, whatever that means. Usually 0. Not much is known about 
  ; this bit. Interestingly, the SPPU1 chip has a pin named "EXTSYNC" (or not-EXTSYNC, since it has a 
  ; bar over it) which is tied to Vcc.

  ; ^^When this bit is set, you may enable BG2 on Mode 7. BG2 uses the same tile and character data as 
  ; BG1, but interprets the high bit of the color data as a priority for the pixel. Various sources 
  ; report additional effects for this bit, possibly related to bit 7. For example, "Enable the Data 
  ; Supplied From the External Lsi.", whatever that means. Of course, maybe that's a typo and it's 
  ; supposed to apply to bit 7 instead.

  ; ^^^This creates a 512-pixel horizontal resolution by taking pixels from the subscreen for the even-
  ; numbered pixels (zero based) and from the main screen for the odd-numbered pixels. Color math 
  ; behaves just as with Mode 5/6 hires. The interlace bit still has no effect. Mosaic operates as 
  ; normal (not like Mode 5/6). The 'subscreen' pixel is clipped (by windows) when the main-screen 
  ; pixel to the LEFT is clipped, not when the one to the RIGHT is clipped as you'd expect. What 
  ; happens with pixel column 0 is unknown. Enabling this bit in Modes 5 or 6 has no effect.

  ; ^^^^When set, 239 lines will be displayed instead of the normal 224. This also means V-Blank will 
  ; occur that much later, and be shorter. All that happens is that extra lines get added to the 
  ; display, and it seems the TV will like to move the display up 8 pixels. Overscan: The bit only 
  ; matters at the very end of the frame, if you change the setting on line 0xE0 before the normal NMI 
  ; trigger point then it's the same as if you had it on all frame. Note that this affects both the 
  ; NMI trigger point and when HDMA stops for the frame. If you turn the bit off at the very beginning 
  ; of scanline X (for 0xE1<=X<=0xF0), NMI will occur on line X and the last HDMA transfer will occur 
  ; on line X-1. However, on my TV at least, the display will remain in the normal no-overscan 
  ; position for lines E1-EC, it will move up only one pixel for line ED, and it will lose vertical 
  ; sync for lines EF-F4! Turning the bit on, only line E1 gives any effect: NMI will occur on line 
  ; E2, although the last HDMA will still occur on line E0. Anything else acts like you left the bit 
  ; off the whole time. Note, however, that if you wait too long after the beginning of the scanline 
  ; then you will get no effect. Even if there is no visible effect, the overscan setting still 
  ; affects VRAM writes. In particular, executing LDA #'-' / STA &#36;2118 / LDA r2133 / STA &#36;2133 
  ; / LDA #'+' / STA &#36;2118 during the E1-F0 period will write only + or only - to VRAM, depending 
  ; on whether the overscan bit was set to 0 or 1.

  ; ^^^^^When set regardless of BG mode, the OBJ will be interlaced (see bit 0 below), and thus will 
  ; appear half-height. Note that this only controls whether obj are drawn as normal or not; the 
  ; interlace signal is only output to the TV based on bit 0 below.

  ; ^^^^^^When set in BG mode 5 (and probably 6), the effective screen height will be 448 (or 478) 
  ; pixles, rather than 224 (or 239). When set in any other mode, the screen will just get a bit 
  ; jumpy. However, toggling the tilemap each field would simulate the increased screen height (much 
  ; like pseudo-hires simulares hires). In hardware, setting this bit makes the SNES output a normal 
  ; interlace signal rather than always forcing one frame.

  ; @see https://wiki.superfamicom.org/backgrounds
  ; @see https://wiki.superfamicom.org/sprites

  MPYL	; Multiplication Result low byte
  MPYM	; Multiplication Result middle byte
  MPYH	; Multiplication Result high byte
  ; 2134 r l+++?
  ; 2135 r m+++?
  ; 2136 r h+++?
  ;         xxxxxxxx xxxxxxxx xxxxxxxx
  ; This is the 2's compliment product of the 16-bit value written to $211b and the 8-bit value most 
  ; recently written to $211c. There is supposedly no important delay. It may not be operative during 
  ; Mode 7 rendering.

  SLHV	; Software Latch for H/V Counter
  ; 2137   b++++
  ;         --------
  ; When read, the H/V counter (as read from $213c and $213d) will be latched to the current X and Y 
  ; position if bit 7 of $4201 is set. The data actually read is open bus.

  OAMDATAREAD	; Data for OAM read
  ; 2138 r w++?- xxxxxxxx

  ; OAM reads are straightforward: the current byte as set in $2102/3 and incremented by reads from 
  ; this register and writes to $2104 will be returned. Note that writes to the lower table are not 
  ; affected so logically. See register $2104 and the section "SPRITES" below for details. Also, note 
  ; that OAM address invalidation probably affects the address read by this register as well.

  VMDATALREAD	; VRAM Data Read low byte
  VMDATAHREAD	; VRAM Data Read high byte
  ; 2139 r l++?-
  ; 213a r h++?-
  ;         xxxxxxxx xxxxxxxx
  ; Simply, this reads data from VRAM. The address is incremented when either $2139 or $213a is read, 
  ; depending on the setting of bit 7 of $2115. Actually, the reading is more complex. When either of 
  ; these registers is read, the appropriate byte from a word-sized buffer is returned. A word from 
  ; VRAM is loaded into this buffer just before the VRAM address is incremented. The actual data read 
  ; and the amount of the increment depend on the low 4 bits of $2115. The effect of this is that a 
  ; 'dummy read' is required after setting $2116-7 before you start getting the actual data. The 
  ; interaction between these registers and $2118/9 is unknown. 
  ; @see https://wiki.superfamicom.org/backgrounds
  ; @see https://wiki.superfamicom.org/sprites

  CGDATAREAD	; CGRAM Data read
  ; 213b r w++?-
  ;         -bbbbbgg gggrrrrr
  ; This reads from CGRAM. Accesses to CGRAM are handled just like accesses to the low table of OAM, 
  ; see $2138 for details. Note that the color values are stored in BGR order. The '-' bit is PPU2 
  ; Open Bus.

  OPHCT	; Horizontal Scanline Location
  OPVCT	; Vertical Scanline Location
  ; 213c r w++++
  ; 213d r w++++
  ;         -------x xxxxxxxx
  ; These values are latched by reading $2137 when bit 7 of $4201 is set, or by clearing-and-setting 
  ; bit 7 of $4201 either by writing $4201 or by pin 6 of Controller Port 2 (the latch occurs on the 1-
  ; >0 transition). Note that the value read is only 9 bits: bits 1-7 of the high byte are PPU2 Open 
  ; Bus. Each register keeps seperate track of whether to return the low or high byte. The high/low 
  ; selector is reset to 'low' when $213f is read (the selector is NOT reset when the counter is 
  ; latched). H Counter values range from 0 to 339, with 22-277 being visible on the screen. V Counter 
  ; values range from 0 to 261 in NTSC mode (262 is possible every other frame when interlace is 
  ; active) and 0 to 311 in PAL mode (312 in interlace?), with 1-224 (or 1-239(?) if overscan is 
  ; enabled) visible on the screen.

  STAT77	; PPU Status Flag and Version
  ; 213e r b++++
  ;         trm-vvvv
  ;         t        = Time Over Flag.^
  ;          r       = Range Over Flag.^^
  ;           m      = "Master/slave mode select".^^^
  ;            -     = PPU1 Open Bus.
  ;             vvvv = 5c77 chip version number. So far, we've only encountered version 1.
  ; ^If more than 34 sprite-tiles (e.g. a 16x16 sprite has 2 sprite-tiles) were encountered on a 
  ; single line, this flag will be set. The flag is reset at the end of V-Blank. 
  ; @see https://wiki.superfamicom.org/sprites

  ; ^^If more than 32 sprites were encountered on a single line, this flag will be set. The flag is 
  ; reset at the end of V-Blank. 
  ; @see https://wiki.superfamicom.org/sprites

  ; ^^^Little is known about this bit. Current theory is that it indicates the status of the "MASTER" 
  ; pin on the S-PPU1 chip, which in the normal SNES is always Gnd. We always seem to read back 0 here.

  STAT78	; PPU Status Flag and Version
  ; 213f r b++++
  ;         fl-pvvvv
  ;         f        = Interlace Field.^
  ;          l       = External latch flag.^^
  ;           -      = PPU2 Open Bus.
  ;            p     = NTSC/Pal Mode.^^^
  ;             vvvv = 5C78 chip version number. So far, we've encountered at least 2 and 3. Possibly 
  ; 1 as well.
  ; ^This will toggle every V-Blank.

  ; ^^When the PPU counters are latched, this flag gets set. The flag is reset on read, but only when 
  ; $4201 bit 7 is set.

  ; ^^^If this is a PAL SNES, this bit will be set, otherwise it will be clear.

  ; Note: as a side effect of reading this register, the high/low byte selector for $213c/d is reset 
  ; to 'low'.

  APUIO0	; APU I/O register 0
  APUIO1	; APU I/O register 1
  APUIO2	; APU I/O register 2
  APUIO3	; APU I/O register 3
  ; 2140 rwb++++
  ; 2141 rwb++++
  ; 2142 rwb++++
  ; 2143 rwb++++
  ;         xxxxxxxx
  ; These registers are used in communication with the SPC700. Note that the value written here is not 
  ; the value read back. Rather, the value written shows up in the SPC700's registers $f4-7, and the 
  ; values written to those registers by the SPC700 are what you read here. If the SPC700 writes the 
  ; register during a read, the value read will be the logical OR of the old and new values. The exact 
  ; cycles during which the 'read' actually occurs is not known, although a good guess would be some 
  ; portion of the final 3 master cycles of the 6-cycle memory access. Note that these registers are 
  ; mirrored throughout the range $2140-$217f.

  WMDATA = $2180	; WRAM Data read/write
  ; 2180 rwb++++
  ;         xxxxxxxx
  ; This register reads to or writes from the WRAM address set in $2181-3. The address is then 
  ; incremented. The effect of mixed reads and writes is unknown, but it is suspected that they are 
  ; handled logically. Note that attempting a DMA from WRAM to this register will not work, WRAM will 
  ; not be written. Attempting a DMA from this register to WRAM will similarly not work, the value 
  ; written is (initially) the Open Bus value. In either case, the address in $2181-3 is not 
  ; incremented.

  WMADDL	; WRAM Address low byte
  WMADDM	; WRAM Address middle byte
  WMADDH	; WRAM Address high bit
  ; 2181  wl++++
  ; 2182  wm++++
  ; 2183  wh++++
  ;         -------x xxxxxxxx xxxxxxxx
  ; This is the address that will be read or written by accesses to $2180. Note that WRAM is also 
  ; mapped in the SNES memory space from $7E:0000 to $7F:FFFF, and from $0000 to $1FFF in banks $00 
  ; through $3F and $80 through $BF. Verious docs indicate that these registers may be read as well as 
  ; written. However, they are wrong. These registers are open bus. DMA from WRAM to these registers 
  ; has no effect. Otherwise, however, DMA writes them as normal. This means you could use DMA mode 4 
  ; to $2180 and a table in ROM to write any sequence of RAM addresses. The value does not wrap at 
  ; page boundaries on increment.

  JOYSER0	= $4016 ; NES-style Joypad Access Port 1
  JOYSER1	= $4017 ; NES-style Joypad Access Port 2
  ; 4016 rwb++++
  ;         Rd: ------ca
  ;         Wr: -------l
  ; 4017 r?b++++
  ;         ---111db
  ;     l    = Writing this bit controlls the Latch line of both controller ports. 
  ;            When 1 is set, the Latch goes high (or is it low? At any rate, whichever one makes the 
  ; pads latch their state). 
  ;            When cleared, the Latch goes the other way.

  ;     a/b  = These bits return the state of the Data1 line.
  ;     c/d  = These bits return the state of the Data2 line.
  ;            Reading &#36;4016 drives the Clock line of Controller Port 1 low.
  ;            The SNES then reads the Data1 and Data2 lines, and Clock is set back to high.
  ;            &#36;4017 does the same for Port 2.
  ; These registers basically have a direct connection to the controller ports on the front of the 
  ; SNES. Note the 1-bits in $4017: the CPU chip has pins for these bits, but these pins are tied to 
  ; Gnd and thus always 1. Data for normal joypads is returned in the order: B, Y, Select, Start, Up, 
  ; Down, Left, Right, A, X, L, R, 0, 0, 0, 0, then ones until latched again. Note that Auto-Joypad 
  ; Read (see register $4200) will effectively write 1 then 0 to bit 'l', then read 16 times from both 
  ; $4016 and $4017. The 'a' bits will end up in $4218/9, with the first bit read (e.g. the B button) 
  ; in bit 15 of the word. Similarly, the 'b' bits end up in $421a/b, the 'c' bits in $42c/d, and the 
  ; 'd' bits in $421e/f. Any further bits the device may return may be read from $4016/$4017 as 
  ; normal. The effect of reading these during auto-joypad read is unknown. 
  ; @see https://wiki.superfamicom.org/controllers

  NMITIMEN = $4200	; Interrupt Enable Flags
  ; 4200  wb+++?
  ;         n-yx---a
  ;         n        = Enable NMI.^
  ;           x/y    = IRQ enable.
  ;                    0/0 => No IRQ will occur
  ;                    0/1 => An IRQ will occur sometime just after the V Counter reaches the value 
  ; set in &#36;4209/a.
  ;                    1/0 => An IRQ will occur sometime just after the H Counter reaches the value 
  ; set in &#36;4207/8.
  ;                    1/1 => An IRQ will occur sometime just after the H Counter reaches the value 
  ; set in &#36;4207/8 when V Counter equals the value set in &#36;4209/a.
  ;                a = Auto-Joypad Read Enable.^^
  ; ^If clear, NMI will not occur. If set, NMI will fire just after the start of V-Blank. NMI fires 
  ; shortly after the V Counter reaches $E1 (or presumably $F0 if overscan is enabled, see register 
  ; $2133).

  ; ^^When set, the registers $4218-$421f will be updated at about V Counter = $E3 (or presumably $F2).

  ; Some games try to read this register. However, they work only because open bus behavior gives them 
  ; values they expect. This register is initialized to $00 on power on or reset.

  WRIO	; Programmable I/O port (out-port)
  ; 4201  wb++++
  ;         abxxxxxx
  ; This is basically just an 8-bit I/O Port. 'b' is connected to pin 6 of Controller Port 1. 'a' is 
  ; connected to pin 6 of Controller Port 2, and to the PPU Latch line. Thus, writing a 0 then a 1 to 
  ; bit 'a' will latch the H and V Counters much like reading $2137 (the latch happens on the 
  ; transition to 0). When bit 'a' is 0, no latching can occur. Any other effects of this register are 
  ; unknown. See $4213 for the I half of the I/O Port. Note that the IO Port is initialized as if this 
  ; register were written with all 1-bits at power up, unchanged on reset(?).

  WRMPYA	; Multiplicand A
  WRMPYB	; Multiplicand B
  ; 4202  wb++++
  ; 4203  wb++++
  ;         mmmmmmmm
  ; Write $4202, then $4203. 8 "machine cycles" (probably 48 master cycles) after $4203 is set, the 
  ; product may be read from $4216/7. $4202 will not be altered by this process, thus a new value may 
  ; be written to $4203 to perform another multiplication without resetting $4202. The multiplication 
  ; is unsigned. $4202 holds the value $ff on power on and is unchanged on reset.

  WRDIVL	; Dividend C low byte
  WRDIVH	; Dividend C high byte
  WRDIVB	; Divisor B
  ; 4204  wl++++
  ; 4205  wh++++
  ;         dddddddd dddddddd
  ; 4206  wb++++
  ;         bbbbbbbb
  ; Write $4204/5, then $4206. 16 "machine cycles" (probably 96 master cycles) after $4206 is set, the 
  ; quotient may be read from $4214/5, and the remainder from $4216/7. Presumably, $4204/5 are not 
  ; altered by this process, much like $4202. The division is unsigned. Division by 0 gives a quotient 
  ; of $FFFF and a remainder of C. WRDIV holds the value $ffff on power on and is unchanged on reset.

  HTIMEL	; H Timer low byte
  HTIMEH	; H Timer high byte
  ; 4207  wl++++
  ; 4208  wh++++
  ;         -------h hhhhhhhh
  ; If bit 4 of $4200 is set and bit 5 is clear, an IRQ will fire every scanline when the H Counter 
  ; reaches the value set here. If bits 4 and 5 are both set, the IRQ will fire only when the V 
  ; Counter equals the value set in $4209/a. Note that the H Counter ranges from 0 to 339, thus 
  ; greater values will result in no IRQ firing. HTIME is initialized to $1ff on power on, unchanged 
  ; on reset.

  VTIMEL	; V Timer low byte
  VTIMEH	; V Timer high byte
  ; 4209  wl++++
  ; 420a  wh++++
  ;         -------v vvvvvvvv
  ; If bit 5 of $4200 is set and bit 4 is clear, an IRQ will fire just after the V Counter reaches the 
  ; value set here. If bits 4 and 5 are both set, the IRQ will fire instead when the V Counter equals 
  ; the value set here and the H Counter reaches the value set in $4207/8. Note that the V Counter 
  ; ranges from 0 to 261 in NTSC mode (262 is possible every other frame whan interlace is active) and 
  ; 0 to 311 in PAL mode (312 in interlace?), thus greater values will result in no IRQ firing. VTIME 
  ; is initialized to $1ff on power on, unchanged on reset.

  MDMAEN	; DMA Enable
  ; 420b  wb++++
  ;         76543210
  ;         7/6/5/4/3/2/1/0 = Enable the selected DMA channels.^
  ; ^The CPU will be paused until all DMAs complete. DMAs will be executed in order from 0 to 7 (?).

  ; See registers $43x0-$43xA for more details. If HDMA (init or transfer) occurs while a DMA is in 
  ; progress, the DMA will be paused for the duration. If the HDMA happens to involve the current DMA 
  ; channel, the DMA will be immediately terminated and the HDMA will progress using the then-current 
  ; values of the registers. Other DMA channels will be unaffected. This register is initialized to 
  ; $00 on power on or reset. 
  ; @see https://wiki.superfamicom.org/dma-and-hdma

  HDMAEN	; HDMA Enable
  ; 420c  wb++++
  ;         76543210
  ;         7/6/5/4/3/2/1/0 = Enable the selected HDMA channels.^
  ; ^HDMAs will be executed in order from 0 to 7 (?).

  ; See registers $43x0-$43xA for more details. If HDMA (init or transfer) occurs while a DMA is in 
  ; progress, the DMA will be paused for the duration. If the HDMA happens to involve the current DMA 
  ; channel, the DMA will be immediately terminated and the HDMA will progress using the then-current 
  ; values of the registers. Other DMA channels will be unaffected. Note that enabling a channel mid-
  ; frame will begin HDMA at the next HDMA point. However, the HDMA register initialization only 
  ; occurs before the HDMA point on scanline 0, so those registers will have to be initialized by hand 
  ; before enabling HDMA. A channel that has already terminated for the frame cannot be restarted in 
  ; this manner. Writing 0 to a bit will pause an ongoing HDMA; the transfer may be continued by 
  ; writing 1 to the bit. This register is initialized to $00 on power on or reset. 
  ; @see https://wiki.superfamicom.org/dma-and-hdma

  MEMSEL	; ROM Access Speed
  ; 420d  wb++++
  ;         -------f
  ;                f = FastROM select.
  ; The SNES uses a master clock running at about 21.477 MHz (current theory is 1.89e9/88 Hz). By 
  ; default, the SNES takes 8 master cycles for each ROM access. If this bit is set and ROM is 
  ; accessed via banks $80-$FF, only 6 master cycles will be used. This register is initialized to $00 
  ; on power on (or reset?). See my memory map and timing doc (memmap.txt) for more details.

  RDNMI = $4210 ; NMI Flag and 5A22 Version
  ; 4210 r b++++ RDNMI
  ;         n---vvvv
  ;         n        = NMI Flag.^
  ;          ---     = Open Bus
  ;             vvvv = 5A22 chip version number.^^
  ; ^This bit is set at the start of V-Blank (at the moment, we suspect when H-Counter is somewhere 
  ; between $28 and $4E), and cleared on read or at the end of V-Blank. Supposedly, it is required 
  ; that this register be read during NMI. Note that this bit is not affected by bit 7 of $4200. NMI 
  ; is cleared on power on or reset.

  ; ^^So far, we've encountered at least 2, maybe 1 as well.

  TIMEUP	; IRQ Flag
  ; 4211 r b++++
  ;         i-------
  ;         i        = IRQ Flag.
  ;          ------- = Open Bus
  ; This bit is set just after an IRQ fires (at the moment, it seems to have the same delay as the NMI 
  ; Flag of $4210 has following NMI), and is cleared on read or write. Supposedly, it is required that 
  ; this register be read during the IRQ handler. If this really is the case, then I suspect that that 
  ; read is what actually clears the CPU's IRQ line. This register is marked read/write in another 
  ; doc, with no explanation. IRQ is cleared on power on or reset.

  HVBJOY	; PPU Status
  ; 4212 r b++++
  ;         vh-----a
  ;         v        = V-Blank Flag.^
  ;          h       = H-Blank Flag.^^
  ;                a = Auto-Joypad Status.^^^
  ; ^If we're currently in V-Blank, this flag is set, otherwise it is clear. The setting seems to 
  ; occur at H Counter about $16-$17 when V Counter is $E1, and the clearing at about $1E with V 
  ; Counter 0.

  ; ^^If we're currently in H-Blank, this flag is set, otherwise it is clear. The setting seems to 
  ; occur at H Counter about $121-$122, and the clearing at about $12-$18.

  ; ^^^This is set while Auto-Joypad Read is in progress, and cleared when complete. It typically 
  ; turns on at the start of V-Blank, and completes 3 scanlines later.

  ; This register is marked read/write in another doc, with no explanation.

  RDIO	; Programmable I/O port (in-port)
  ; 4213 r b++++
  ;         abxxxxxx
  ; Reading this register reads data from the I/O Port. The way the I/O Port works, any bit set to 0 
  ; in $4201 will be 0 here. Any bit set to 1 in $4201 may be 1 or 0 here, depending on whether any 
  ; other device connected to the I/O Port has set a 0 to that bit. Bit 'b' is connected to pin 6 of 
  ; Controller Port 1. Bit 'a' is connected to pin 6 of Controller Port 2, and to the PPU Latch line. 
  ; See register $4201 for the O side of the I/O Port.

  RDDIVL	; Quotient of Divide Result low byte
  RDDIVH	; Quotient of Divide Result high byte
  ; 4214 r l++++
  ; 4215 r h++++
  ;         qqqqqqqq qqqqqqqq

  ;     Write &#36;4204/5, then &#36;4206. 16 "machine cycles" (probably 96 master
  ;     cycles) after &#36;4206 is set, the quotient may be read from these
  ;     registers, and the remainder from &#36;4216/7.

  ;     The division is unsigned.
  RDMPYL	; Multiplication Product or Divide Remainder low byte
  RDMPYH	; Multiplication Product or Divide Remainder high byte
  ; 4216 r l++++
  ; 4217 r h++++
  ;         xxxxxxxx xxxxxxxx
  ; Write $4202, then $4203. 8 "machine cycles" (probably 48 master cycles) after $4203 is set, the 
  ; product may be read from these registers. Write $4204/5, then $4206. 16 "machine cycles" (probably 
  ; 96 master cycles) after $4206 is set, the quotient may be read from $4214/5, and the remainder 
  ; from these registers. The multiplication and division are both unsigned.

  JOY1L	; Controller Port 1 Data1 Register low byte
  JOY1H	; Controller Port 1 Data1 Register high byte
  JOY2L	; Controller Port 2 Data1 Register low byte
  JOY2H	; Controller Port 2 Data1 Register high byte
  JOY3L	; Controller Port 1 Data2 Register low byte
  JOY3H	; Controller Port 1 Data2 Register high byte
  JOY4L	; Controller Port 2 Data2 Register low byte
  JOY4H	; Controller Port 2 Data2 Register high byte
  ; 4218 r l++++
  ; 4219 r h++++
  ; 421a r l++++
  ; 421b r h++++
  ; 421c r l++++
  ; 421d r h++++
  ; 421e r l++++
  ; 421f r h++++
  ;         byetUDLR axlr0000
  ;         a/b/x/y/l/r/e/t   = A/B/X/Y/L/R/Select/Start button status.
  ;         U/D/L/R           = Up/Down/Left/Right control pad status.
  ;                             Note that only one of L/R and only one of U/D may be set, due to the 
  ; pad hardware.
  ; The bitmap above only applies for joypads, obviously. More generically, Auto Joypad Read 
  ; effectively sets 1 then 0 to $4016, then reads $4016/7 16 times to get the bits for these 
  ; registers. These registers are only updated when the Auto-Joypad Read bit (bit 0) of $4200 is set. 
  ; They are being updated while the Auto-Joypad Status bit (bit 0) of $4212 is set. Reading during 
  ; this time will return incorrect values. 
  ; @see https://wiki.superfamicom.org/controllers

  DMAP0 = $4300	; DMA Control for Channel 0
  DMAP1 = $4310	; DMA Control for Channel 1
  DMAP2 = $4320	; DMA Control for Channel 2 
  DMAP3 = $4330	; DMA Control for Channel 3 
  DMAP4 = $4340	; DMA Control for Channel 4 
  DMAP5 = $4350	; DMA Control for Channel 5 
  DMAP6 = $4360	; DMA Control for Channel 6 
  DMAP7 = $4370	; DMA Control for Channel 7 
  ; 43x0 rwb++++
  ;         da-ifttt
  ;         d        = Transfer Direction.^
  ;          a       = HDMA Addressing Mode.^^
  ;            i     = DMA Address Increment.^^^
  ;             f    = DMA Fixed Transfer.^^^^

  ;         ttt  = Transfer Mode.
  ;             000 => 1 register write once             (1 byte:  p               )
  ;             001 => 2 registers write once            (2 bytes: p, p+1          )
  ;             010 => 1 register write twice            (2 bytes: p, p            )
  ;             011 => 2 registers write twice each      (4 bytes: p, p,   p+1, p+1)
  ;             100 => 4 registers write once            (4 bytes: p, p+1, p+2, p+3)
  ;             101 => 2 registers write twice alternate (4 bytes: p, p+1, p,   p+1)
  ;             110 => 1 register write twice            (2 bytes: p, p            )
  ;             111 => 2 registers write twice each      (4 bytes: p, p,   p+1, p+1)
  ; ^When clear, data will be read from the CPU memory and written to the PPU register. When set, vice 
  ; versa. Contrary to previous belief, this bit DOES affect HDMA! Indirect mode is more useful, it 
  ; will read the table as normal and write from Bus B to the Bus A address specified. Direct mode 
  ; will work as expected though, it will read counts from the table and try to write the data values 
  ; into the table.

  ; ^^When clear, the HDMA table contains the data to transfer. When set, the HDMA table contains 
  ; pointers to the data. This bit does not affect DMA.

  ; ^^^When clear, the DMA address will be incremented for each byte. When set, the DMA address will 
  ; be decremented. This bit does not affect HDMA.

  ; ^^^^When set, the DMA address will not be adjusted. When clear, the address will be adjusted as 
  ; specified by bit 4. This bit does not affect HDMA.

  ; The effect of writing this register during HDMA to the associated channel is unknown. Most likely, 
  ; the change takes effect for the next HDMA transfer. This register is set to $ff on power on, and 
  ; is unchanged on reset. 
  ; @see https://wiki.superfamicom.org/dma-and-hdma

  BBAD0 = $4301	; DMA Destination Register for Channel 0
  BBAD1 = $4311	; DMA Destination Register for Channel 1
  BBAD2 = $4321	; DMA Destination Register for Channel 2
  BBAD3 = $4331	; DMA Destination Register for Channel 3
  BBAD4 = $4341	; DMA Destination Register for Channel 4
  BBAD5 = $4351	; DMA Destination Register for Channel 5
  BBAD6 = $4361	; DMA Destination Register for Channel 6
  BBAD7 = $4371	; DMA Destination Register for Channel 7
  ; 43x1 rwb++++
  ;         pppppppp
  ; This specifies the Bus B address to access. Considering the standard CPU memory space, this 
  ; specifies which address $00:2100-$00:21ff to access, with two- and four-register modes wrapping 
  ; $21ff->$2100, not $2200. The effect of writing this register during HDMA to the associated channel 
  ; is unknown. Most likely, the change takes effect for the next transfer. This register is set to 
  ; $ff on power on, and is unchanged on reset. 
  ; @see https://wiki.superfamicom.org/dma-and-hdma

  A1T0L	= $4302 ; DMA Source Address for Channel 0 low byte
  A1T0H	= $4303 ; DMA Source Address for Channel 0 high byte
  A1B0	= $4304 ; DMA Source Address for Channel 0 bank byte
  A1T1L	= $4312 ; DMA Source Address for Channel 1 low byte
  A1T1H	= $4313 ; DMA Source Address for Channel 1 high byte
  A1B1	= $4314 ; DMA Source Address for Channel 1 bank byte
  A1T2L	= $4322 ; DMA Source Address for Channel 2 low byte
  A1T2H	= $4323 ; DMA Source Address for Channel 2 high byte
  A1B2	= $4324 ; DMA Source Address for Channel 2 bank byte
  A1T3L	= $4332 ; DMA Source Address for Channel 3 low byte
  A1T3H	= $4333 ; DMA Source Address for Channel 3 high byte
  A1B3	= $4334 ; DMA Source Address for Channel 3 bank byte
  A1T4L	= $4342 ; DMA Source Address for Channel 4 low byte
  A1T4H	= $4343 ; DMA Source Address for Channel 4 high byte
  A1B4	= $4344 ; DMA Source Address for Channel 4 bank byte
  A1T5L	= $4352 ; DMA Source Address for Channel 5 low byte
  A1T5H	= $4353 ; DMA Source Address for Channel 5 high byte
  A1B5	= $4354 ; DMA Source Address for Channel 5 bank byte
  A1T6L	= $4362 ; DMA Source Address for Channel 6 low byte
  A1T6H	= $4363 ; DMA Source Address for Channel 6 high byte
  A1B6	= $4364 ; DMA Source Address for Channel 6 bank byte
  A1T7L	= $4372 ; DMA Source Address for Channel 7 low byte
  A1T7H	= $4373 ; DMA Source Address for Channel 7 high byte
  A1B7	= $4374 ; DMA Source Address for Channel 7 bank byte
  ; 43x2 rwl++++
  ; 43x3 rwh++++
  ; 43x4 rwb++++
  ;         bbbbbbbb hhhhhhhh llllllll
  ; This specifies the starting Address Bus A address for the DMA transfer, or the beginning of the 
  ; HDMA table for HDMA transfers. Note that Bus A does not access the Bus B registers, so pointing 
  ; this address at say $00:2100 results in open bus. The effect of writing this register during HDMA 
  ; to the associated channel is unknown. However, current theory is that only $43x4 will affect the 
  ; transfer. The changes will take effect at the next HDMA init. During DMA, $43x2/3 will be 
  ; incremented or decremented as specified by $43x0. However $43x4 will NOT be adjusted. These 
  ; registers will not be affected by HDMA. This register is set to $ff on power on, and is unchanged 
  ; on reset. 
  ; @see https://wiki.superfamicom.org/dma-and-hdma

  DAS0L = $4305	; DMA Size/HDMA Indirect Address low byte
  DAS0H = $4306	; DMA Size/HDMA Indirect Address high byte
  DASB0 = $4307	; HDMA Indirect Address bank byte
  DAS1L = $4315	; DMA Size/HDMA Indirect Address low byte
  DAS1H = $4316	; DMA Size/HDMA Indirect Address high byte
  DASB1 = $4317	; HDMA Indirect Address bank byte
  DAS2L = $4325	; DMA Size/HDMA Indirect Address low byte
  DAS2H = $4326	; DMA Size/HDMA Indirect Address high byte
  DASB2 = $4327	; HDMA Indirect Address bank byte
  DAS3L = $4335	; DMA Size/HDMA Indirect Address low byte
  DAS3H = $4336	; DMA Size/HDMA Indirect Address high byte
  DASB3 = $4337	; HDMA Indirect Address bank byte
  DAS4L = $4345	; DMA Size/HDMA Indirect Address low byte
  DAS4H = $4346	; DMA Size/HDMA Indirect Address high byte
  DASB4 = $4347	; HDMA Indirect Address bank byte
  DAS5L = $4355	; DMA Size/HDMA Indirect Address low byte
  DAS5H = $4356	; DMA Size/HDMA Indirect Address high byte
  DASB5 = $4357	; HDMA Indirect Address bank byte
  DAS6L = $4365	; DMA Size/HDMA Indirect Address low byte
  DAS6H = $4366	; DMA Size/HDMA Indirect Address high byte
  DASB6 = $4367	; HDMA Indirect Address bank byte
  DAS7L = $4375	; DMA Size/HDMA Indirect Address low byte
  DAS7H = $4376	; DMA Size/HDMA Indirect Address high byte
  DASB7 = $4377	; HDMA Indirect Address bank byte
  ; 43x5 rwl++++
  ; 43x6 rwh++++
  ; 43x7 rwb++++
  ;         bbbbbbbb hhhhhhhh llllllll
  ; For DMA, $43x5/6 indicate the number of bytes to transfer. Note that this is a strict limit: if 
  ; this is set to 1 then only 1 byte will be written, even if the transfer mode specifies 2 or 4 
  ; registers (and if this is 5, all 4 registers would be written once, then the first only would be 
  ; written a second time). Note, however, that writing $0000 to this register actually results in a 
  ; transfer of $10000 bytes, not 0. $43x5/6 are decremented during DMA, and thus typically end up set 
  ; to 0 when DMA is complete. For HDMA, $43x7 specifies the bank for indirect addressing mode. The 
  ; indirect address is copied into $43x5/6 and incremented appropriately. For direct HDMA, these 
  ; registers are not used or altered. Writes to $43x7 during indirect HDMA will take effect for the 
  ; next transfer. Writes to $43x5/6 during indirect HDMA will also take effect for the next HDMA 
  ; transfer, however this is only noticeable during repeat mode (for normal mode, a new indirect 
  ; address will be read from the table before the transfer). For a direct transfer, presumably 
  ; nothing will happen. This register is set to $ff on power on, and is unchanged on reset. 
  ; @see https://wiki.superfamicom.org/dma-and-hdma

  A2A0L = $4308	; HDMA Table Address low byte
  A2A0H = $4309	; HDMA Table Address high byte
  A2A1L = $4318	; HDMA Table Address low byte
  A2A1H = $4319	; HDMA Table Address high byte
  A2A2L = $4328	; HDMA Table Address low byte
  A2A2H = $4329	; HDMA Table Address high byte
  A2A3L = $4338	; HDMA Table Address low byte
  A2A3H = $4339	; HDMA Table Address high byte
  A2A4L = $4348	; HDMA Table Address low byte
  A2A4H = $4349	; HDMA Table Address high byte
  A2A5L = $4358	; HDMA Table Address low byte
  A2A5H = $4359	; HDMA Table Address high byte
  A2A6L = $4368	; HDMA Table Address low byte
  A2A6H = $4369	; HDMA Table Address high byte
  A2A7L = $4378	; HDMA Table Address low byte
  A2A7H = $4379	; HDMA Table Address high byte
  ; 43x8 rwl++++
  ; 43x9 rwh++++
  ;         aaaaaaaa aaaaaaaa
  ; At the beginning of the frame $43x2/3 are copied into this register for all active HDMA channels, 
  ; and then this register is updated as the table is read. Thus, if a game wishes to start HDMA mid-
  ; frame (or change tables mid-frame), this register must be written. Writing this register mid-frame 
  ; changes the table address for the next scanline. This register is not used for DMA. This register 
  ; is set to $ff on power on, and is unchanged on reset. 
  ; @see https://wiki.superfamicom.org/dma-and-hdma

  NLTR0 = $430a	; HDMA Line Counter
  NLTR1 = $431a	; HDMA Line Counter
  NLTR2 = $432a	; HDMA Line Counter
  NLTR3 = $433a	; HDMA Line Counter
  NLTR4 = $434a	; HDMA Line Counter
  NLTR5 = $435a	; HDMA Line Counter
  NLTR6 = $436a	; HDMA Line Counter
  NLTR7 = $437a	; HDMA Line Counter
  ; 43xa rwb++++
  ;         rccccccc
  ;         r        = Repeat Select.^
  ;          ccccccc = Line count.^^
  ; ^When set, the HDMA transfer will be performed every line, rather than only when this register is 
  ; loaded from the table. However, this byte (and the indirect HDMA address) will only be reloaded 
  ; from the table when the counter reaches 0.

  ; ^^This is decremented every scanline. When it reaches 0, a byte is read from the HDMA table into 
  ; this register (and the indirect HDMA address is read into $43x5/6 if applicable).

  ; One oddity: the register is decremeted before being checked for r status or c==0. Thus, setting a 
  ; value of $80 is really "128 lines with no repeat" rather than "0 lines with repeat". Similarly, a 
  ; value of $00 will be "128 lines with repeat" when it doesn't mean "terminate the channel". This 
  ; register is initialized at the end of V-Blank for every active HDMA channel. Note that if a game 
  ; wishes to begin HDMA during the frame, it will most likely have to initalize this register. 
  ; Writing this mid-transfer will similarly change the count and repeat to take effect next scanline. 
  ; Remember though that 'repeat' won't take effect until after the next transfer period. This 
  ; register is set to $ff on power on, and is unchanged on reset. 
  ; @see https://wiki.superfamicom.org/dma-and-hdma

  ; ????x - Unknown (x=0-7)
  ; ????x - Unknown (x=0-7)
  ; 43xb rwb++++
  ; 43xf rwb++++
  ;         ????????
  ; The effects of these registers (if any) are unknown. $43xf and $43xb are really aliases for the 
  ; same register. This register is set to $ff on power on, and is unchanged on reset.
  .endenum

  ; DMA mirror registers
  .enum DMA
  ; Channel 0
  ; ----------
  MODE0 = SFC::DMAP0
  DST0  = SFC::BBAD0
  SRC0  = SFC::A1T0L
  BNK0  = SFC::A1B0
  LEN0  = SFC::DAS0L
  ; Channel 1
  ; ----------
  MODE1 = SFC::DMAP1
  DST1  = SFC::BBAD1
  SRC1  = SFC::A1T1L
  BNK1  = SFC::A1B1
  LEN1  = SFC::DAS1L
  ; Channel 2
  ; ----------
  MODE2 = SFC::DMAP2
  DST2  = SFC::BBAD2
  SRC2  = SFC::A1T2L
  BNK2  = SFC::A1B2
  LEN2  = SFC::DAS2L
  ; Channel 3
  ; ----------
  MODE3 = SFC::DMAP3
  DST3  = SFC::BBAD3
  SRC3  = SFC::A1T3L
  BNK3  = SFC::A1B3
  LEN3  = SFC::DAS3L
  ; Channel 4
  ; ----------
  MODE4 = SFC::DMAP4
  DST4  = SFC::BBAD4
  SRC4  = SFC::A1T4L
  BNK4  = SFC::A1B4
  LEN4  = SFC::DAS4L
  ; Channel 5
  ; ----------
  MODE5 = SFC::DMAP5
  DST5  = SFC::BBAD5
  SRC5  = SFC::A1T5L
  BNK5  = SFC::A1B5
  LEN5  = SFC::DAS5L
  ; Channel 6
  ; ----------
  MODE6 = SFC::DMAP6
  DST6  = SFC::BBAD6
  SRC6  = SFC::A1T6L
  BNK6  = SFC::A1B6
  LEN6  = SFC::DAS6L
  ; Channel 7
  ; ----------
  MODE7 = SFC::DMAP7
  DST7  = SFC::BBAD7
  SRC7  = SFC::A1T7L
  BNK7  = SFC::A1B7
  LEN7  = SFC::DAS7L
  .endenum

  ; Sprites sizes for OBSEL
  .enum OB
  SIZE_8_16  = $00
  SIZE_8_32  = $20
  SIZE_8_64  = $40
  SIZE_16_32 = $60
  SIZE_16_64 = $80
  SIZE_32_64 = $A0
  .endenum

  ; Screens for TM
  .enum TM
  BG1 = (1 << 0)
  BG2 = (1 << 1)
  BG3 = (1 << 2)
  BG4 = (1 << 3)
  OBJ = (1 << 4)
  .endenum

  ; Options for M7SEL
  .enum M7
  HFLIP    = $01
  VFLIP    = $02
  WRAP     = $00
  NOWRAP   = $80
  BORDER00 = $C0
  .endenum

  ; Options for VMAIN
  .enum VM
  INC_BY_1
  INC_BY_32
  INC_BY_128
  INC_BY_128b
  INC_DATAHI  = $80
  .endenum

  ; Options for NMITIMEN
  .enum NMI
  ENABLE =     $80
  HTIME_IRQ  = $10
  VTIME_IRQ  = $20
  HVTIME_IRQ = $30
  JOY_READ   = $01
  .endenum

  ; Values read from JOYxL and JOYxH
  .enum JOY
  KEY_B      = $8000
  KEY_Y      = $4000
  KEY_SELECT = $2000
  KEY_START  = $1000
  KEY_UP     = $0800
  KEY_DOWN   = $0400
  KEY_LEFT   = $0200
  KEY_RIGHT  = $0100
  KEY_A      = $0080
  KEY_X      = $0040
  KEY_L      = $0020
  KEY_R      = $0010
  .endenum
;--------------------------------------------------------------------------------------------------

  ; Macros
  ; ======

  .macro seti8
  sep #$10
  .endmacro

  .macro seti16
  rep #$10
  .endmacro

  .macro seta8
  sep #$20
  .endmacro

  .macro seta16
  rep #$20
  .endmacro

  .macro setai8
  sep #$30
  .endmacro

  .macro setai16
  rep #$30
  .endmacro

  .feature leading_dot_in_identifiers

  .macro .a8i8
  .a8
  .i8
  .endmac

  .macro .a8i16
  .a8
  .i16
  .endmac

  .macro .a16i8
  .a16
  .i8
  .endmac

  .macro .a16i16
  .a16
  .i16
  .endmac

  .macro PPU_VSync
  .local wait_clr, wait_set
wait_clr:
    bit SFC::HVBJOY  ; Wait for leaving previous vblank
    bmi wait_clr
wait_set:
    bit SFC::HVBJOY  ; Wait for start of this vblank
    bpl wait_set
  .endmac

  .macro Joy_WaitPoll
  .local wait_pad
  lda #1
wait_pad:
  bit SFC::HVBJOY
  bne wait_pad
  .endmac

  .define RGB(r,g,b) ((r)|((g)<<5)|((b)<<10))

;--------------------------------------------------------------------------------------------------

  ; Structs
  ; =======

  .struct Joy
  curState     .res 2*2
  prvState     .res 2*2
  .endstruct

joy_curState = joy+Joy::curState
joy_prvState = joy+Joy::prvState

  .struct OAM
    .struct Object
    xoff    .res 1
    yoff    .res 1
    tile    .res 1
    attr    .res 1
    .endstruct
  lo        .res .sizeof(Object)*128
  hi        .res .sizeof(Object)*128
  idx       .res 2
  dmaReady  .res 1
  .endstruct

oam_x =         oam+OAM::lo+OAM::Object::xoff
oam_y =         oam+OAM::lo+OAM::Object::yoff
oam_tile =      oam+OAM::lo+OAM::Object::tile
oam_attr =      oam+OAM::lo+OAM::Object::attr
oam_lo =        oam+OAM::lo
oam_hi =        oam+OAM::hi
oam_idx =       oam+OAM::idx
oam_dmaReady =  oam+OAM::dmaReady

;--------------------------------------------------------------------------------------------------

  ; Variables
  ; =========

  .globalzp joy
  .global oam

;--------------------------------------------------------------------------------------------------

  ; Functions
  ; =========

  ; OAM functions
  ; -------------
  .global OAM_update
  .global OAM_compactHiTable
  .global OAM_hideUnusedSprites
  .global OAM_doDMA

  ; Joypad functions
  ; ----------------
  .global Joy_update

;--------------------------------------------------------------------------------------------------