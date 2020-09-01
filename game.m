Modules list:
-------------
sfc.o:
    CODE              Offs=000000  Size=000150  Align=00001  Fill=0000
    BSS               Offs=000000  Size=000407  Align=00001  Fill=0000
    ZEROPAGE          Offs=000000  Size=000008  Align=00001  Fill=0000
    SFCHEADER         Offs=000000  Size=000050  Align=00001  Fill=0000
    CODE0             Offs=000000  Size=000011  Align=00001  Fill=0000
game.o:
    CODE              Offs=000150  Size=00004A  Align=00001  Fill=0000


Segment list:
-------------
Name                   Start     End    Size  Align
----------------------------------------------------
ZEROPAGE              000000  000007  000008  00001
BSS                   000200  000606  000407  00001
CODE0                 808000  808010  000011  00001
SFCHEADER             80FFB0  80FFFF  000050  00001
CODE                  878000  878199  00019A  00001


Exports list by name:
---------------------
Game_irq                  878199 RLA    Game_main                 878150 RLA    
Game_nmi                  87817E RLA    Joy_update                87812B RLA    
OAM_update                8780A1 RLA    __MAPPER__                000020 REA    
__REGION__                000001 REA    __ROMSIZE__               000008 REA    
__ROMSPEED__              000010 REA    __SRAMINSIDE__            000000 REA    
__SRAMSIZE__              000000 REA    __WRAMSIZE__              000000 REA    
__ZEROPAGE_RUN__          000000 RLA    joy                       000000 RLZ    
oam                       000200 RLA    


Exports list by value:
----------------------
__SRAMINSIDE__            000000 REA    __SRAMSIZE__              000000 REA    
__WRAMSIZE__              000000 REA    __ZEROPAGE_RUN__          000000 RLA    
joy                       000000 RLZ    __REGION__                000001 REA    
__ROMSIZE__               000008 REA    __ROMSPEED__              000010 REA    
__MAPPER__                000020 REA    oam                       000200 RLA    
OAM_update                8780A1 RLA    Joy_update                87812B RLA    
Game_main                 878150 RLA    Game_nmi                  87817E RLA    
Game_irq                  878199 RLA    


Imports list:
-------------
Game_irq (game.o):
    sfc.o                     /home/paris/sfc/ca65/sfc_template/src/sfc.s(11)
Game_main (game.o):
    sfc.o                     /home/paris/sfc/ca65/sfc_template/src/sfc.s(11)
Game_nmi (game.o):
    sfc.o                     /home/paris/sfc/ca65/sfc_template/src/sfc.s(11)
Joy_update (sfc.o):
    game.o                    /home/paris/sfc/ca65/sfc_template/inc/sfc.inc(1401)
OAM_update (sfc.o):
    game.o                    /home/paris/sfc/ca65/sfc_template/inc/sfc.inc(1394)
__MAPPER__ ([linker generated]):
    sfc.o                     /home/paris/sfc/ca65/sfc_template/src/sfc.s(4)
__REGION__ ([linker generated]):
    sfc.o                     /home/paris/sfc/ca65/sfc_template/src/sfc.s(5)
__ROMSIZE__ ([linker generated]):
    sfc.o                     /home/paris/sfc/ca65/sfc_template/src/sfc.s(6)
__ROMSPEED__ ([linker generated]):
    sfc.o                     /home/paris/sfc/ca65/sfc_template/src/sfc.s(7)
__SRAMINSIDE__ ([linker generated]):
    sfc.o                     /home/paris/sfc/ca65/sfc_template/src/sfc.s(10)
__SRAMSIZE__ ([linker generated]):
    sfc.o                     /home/paris/sfc/ca65/sfc_template/src/sfc.s(9)
__WRAMSIZE__ ([linker generated]):
    sfc.o                     /home/paris/sfc/ca65/sfc_template/src/sfc.s(8)
__ZEROPAGE_RUN__ ([linker generated]):
    sfc.o                     /home/paris/sfc/ca65/sfc_template/src/sfc.s(3)
joy (sfc.o):
    game.o                    /home/paris/sfc/ca65/sfc_template/inc/sfc.inc(1384)
oam (sfc.o):
    game.o                    /home/paris/sfc/ca65/sfc_template/inc/sfc.inc(1385)

