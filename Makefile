ROMNAME = game.sfc
BOARD = lorom256kb

PROJECT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
PROJECT_DIR := $(PROJECT_DIR:/=)
PROJECT_SRC = $(PROJECT_DIR)/src
PROJECT_OBJ = $(PROJECT_DIR)/obj
PROJECT_INC = $(PROJECT_DIR)/inc
PROJECT_CFG = $(PROJECT_DIR)/cfg
EMUEXE = mesen-s
EMUOPT = 2>/dev/null
MAP = $(ROMNAME:.sfc=.m)
DBG = $(ROMNAME:.sfc=.dbg)
CA = ca65
LD = ld65
AFLAGS = -I $(PROJECT_INC) -g --large-alignment

rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
SOURCES = $(call rwildcard, $(PROJECT_SRC), *.s)
CFGS = $(shell find $(PROJECT_CFG) -name '$(BOARD).cfg')
CFGS := $(if $(CFGS), $(CFGS), $(shell find $(PROJECT_CFG) -name '$(BOARD)-prg*.cfg'|sort))
OBJS := $(SOURCES:.s=.o)
OBJS := $(OBJS:$(PROJECT_SRC)/%=$(PROJECT_OBJ)/%)

ifeq ($(shell echo),)
	MKDIR = mkdir -p $1
else
	MKDIR = mkdir $(subst /,\,$1)
endif

$(ROMNAME): $(CFGS) $(SOURCES) $(OBJS) 
	$(LD) -C $(CFGS) --dbgfile $(DBG) -m $(MAP) -o $@ $(OBJS)

all: clean alloc play

play: $(ROMNAME)
	$(EMUEXE) $(ROMNAME) $(EMUOPT)

.SECONDEXPANSION:

$(PROJECT_OBJ)/%.o: $(PROJECT_OBJ)/%.s | $$(@D)/.
	$(CA) $(AFLAGS) $< -o $@

$(PROJECT_OBJ)/%.o: $(PROJECT_SRC)/%.s | $$(@D)/.
	$(CA) $(AFLAGS) $< -o $@

.PRECIOUS: $(PROJECT_OBJ)/. $(PROJECT_OBJ)%/.

$(PROJECT_OBJ)/.:
	$(call MKDIR,$@)

$(PROJECT_OBJ)%/.:
	$(call MKDIR,$@)

.PHONY: alloc clean

alloc:
	# java -jar res/tool/pa65.jar -d -t -o $(PROJECT_INC)/pa65.inc $(SOURCES)

clean:
	$(RM) $(OBJS) $(DBG) $(MAP) $(ROMNAME)
