# Makefile for cc65 Apple II target

ROOT_DIR   = $(CURDIR)
SRC_DIR    = $(ROOT_DIR)/src
ASM_DIR    = $(ROOT_DIR)/asm
INCLUDE_DIR= $(ROOT_DIR)/include
CONFIG_DIR = $(ROOT_DIR)/config
ASSETS_DIR = $(ROOT_DIR)/assets
TOOLS_DIR  = $(ROOT_DIR)/tools
BUILD_DIR ?= $(ROOT_DIR)/build
LOADER_DIR = $(ROOT_DIR)/asm/loader
BASE_DISK  := $(ASSETS_DIR)/iimperialism.dsk
RWTS_DISK  := $(ASSETS_DIR)/iimperialism-rwts.dsk
DISK_BACKEND ?= prodos

ifeq ($(OS),Windows_NT)
CLEAN_CMD = powershell.exe -NoProfile -Command "if (Test-Path '$(BUILD_DIR)') { Remove-Item -Recurse -Force -ErrorAction SilentlyContinue '$(BUILD_DIR)' }; Remove-Item -Force -ErrorAction SilentlyContinue '$(SRC_DIR)/*.o','$(ASM_DIR)/*.o'"
else
MKDIR_P  = mkdir -p
CLEAN_CMD = rm -rf $(BUILD_DIR) $(SRC_DIR)/*.o $(ASM_DIR)/*.o
endif

C_SOURCES  = \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/production.c \
	$(SRC_DIR)/strings.c \
	$(SRC_DIR)/ui_buffers.c \
	$(SRC_DIR)/ui.c \
	$(SRC_DIR)/gamestate.c \
	$(SRC_DIR)/random.c \
	$(SRC_DIR)/logic.c \
	$(SRC_DIR)/overlay.c \
	$(SRC_DIR)/ovl_industry.c \
	$(SRC_DIR)/ovl_production.c \
	$(SRC_DIR)/ovl_transport.c \
	$(SRC_DIR)/ovl_admiralty.c \
	$(SRC_DIR)/ovl_diplomacy.c \
	$(SRC_DIR)/trade_expedition.c \
	$(SRC_DIR)/ovl_trade_expedition_action.c \
	$(SRC_DIR)/ovl_battle.c \
	$(SRC_DIR)/ovl_science.c \
	$(SRC_DIR)/ovl_game_menu.c \
	$(SRC_DIR)/ovl_council_nations.c

ASM_SOURCES = \
	$(ASM_DIR)/werner.s \
	$(ASM_DIR)/text_hgr.s \
	$(ASM_DIR)/ui_wait.s \
	$(ASM_DIR)/sound.s \
	$(ASM_DIR)/jmptab.s

MAIN_OBJECTS = \
	$(BUILD_DIR)/main.o \
	$(BUILD_DIR)/production.o \
	$(BUILD_DIR)/strings.o \
	$(BUILD_DIR)/ui_buffers.o \
	$(BUILD_DIR)/ui.o \
	$(BUILD_DIR)/werner.o \
	$(BUILD_DIR)/text_hgr.o \
	$(BUILD_DIR)/ui_wait.o \
	$(BUILD_DIR)/sound.o \
	$(BUILD_DIR)/gamestate.o \
	$(BUILD_DIR)/random.o \
	$(BUILD_DIR)/logic.o \
	$(BUILD_DIR)/trade_expedition.o \
	$(BUILD_DIR)/overlay.o \
	$(BUILD_DIR)/disk_overlay_load_$(DISK_BACKEND).o \
	$(BUILD_DIR)/jmptab.o

OVERLAY_OBJECTS = \
	$(BUILD_DIR)/ovl_industry.o \
	$(BUILD_DIR)/ovl_production.o \
	$(BUILD_DIR)/ovl_transport.o \
	$(BUILD_DIR)/ovl_admiralty.o \
	$(BUILD_DIR)/ovl_diplomacy.o \
	$(BUILD_DIR)/ovl_trade_expedition_action.o \
	$(BUILD_DIR)/ovl_battle.o \
	$(BUILD_DIR)/ovl_science.o \
	$(BUILD_DIR)/ovl_game_menu.o \
	$(BUILD_DIR)/ovl_council_nations.o

# Main compiler

CC      = cl65
CFLAGS  = -t apple2 -Osr -I$(INCLUDE_DIR) $(EXTRA_CFLAGS)
LDFLAGS = -t apple2 -C $(CONFIG_DIR)/apple2-hgr.cfg -Oirs $(EXTRA_LDFLAGS)

# Overlay linker: raw 2KB binary, no startup, symbols from jump table
OVL_CC = cl65
OVL_CFG = $(BUILD_DIR)/apple2-ovl.cfg
OVL_LDFLAGS = -t apple2 -C $(OVL_CFG) -Oirs

# Disk image tool (AppleCommander)
AC = java -jar $(TOOLS_DIR)/ac.jar
DISK ?= $(BASE_DISK)
LOADER_SYSTEM = IIMP.SYSTEM
MAIN_DISK_NAME = IIMP

all: $(BUILD_DIR) iimperialism overlays $(BUILD_DIR)/loader.system

.PHONY: all disk overlays iimperialism clean memory-usage disk-rwts prepare-rwts-disk patch-rwts-boot

# Update disk image with latest binaries (run after 'make all')
disk: iimperialism overlays $(BUILD_DIR)/loader.system
ifeq ($(DISK_BACKEND),rwts)
	-$(AC) -d $(DISK) IIMP.SYSTEM
	-$(AC) -d $(DISK) PRODOS
else
	-$(AC) -d $(DISK) $(LOADER_SYSTEM)
	$(AC) -p $(DISK) $(LOADER_SYSTEM) SYS < $(BUILD_DIR)/loader.system
endif
	-$(AC) -d $(DISK) $(MAIN_DISK_NAME)
	-$(AC) -d $(DISK) IIMPERIALISM
	$(AC) -p $(DISK) $(MAIN_DISK_NAME) BIN 0x0803 < $(BUILD_DIR)/iimperialism
	-$(AC) -d $(DISK) ISCR
	$(AC) -p $(DISK) ISCR BIN 0x8800 < $(BUILD_DIR)/iscr.bin
	-$(AC) -d $(DISK) PSCR
	$(AC) -p $(DISK) PSCR BIN 0x9000 < $(BUILD_DIR)/pscr.bin
	-$(AC) -d $(DISK) TSCR
	$(AC) -p $(DISK) TSCR BIN 0x9800 < $(BUILD_DIR)/tscr.bin
	-$(AC) -d $(DISK) ASCR
	$(AC) -p $(DISK) ASCR BIN 0xA000 < $(BUILD_DIR)/ascr.bin
	-$(AC) -d $(DISK) DSCR
	$(AC) -p $(DISK) DSCR BIN 0x8800 < $(BUILD_DIR)/dscr.bin
	-$(AC) -d $(DISK) TXAC
	$(AC) -p $(DISK) TXAC BIN 0x8800 < $(BUILD_DIR)/txac.bin
	-$(AC) -d $(DISK) BSCR
	$(AC) -p $(DISK) BSCR BIN 0x8800 < $(BUILD_DIR)/bscr.bin
	-$(AC) -d $(DISK) SSCR
	$(AC) -p $(DISK) SSCR BIN 0x8800 < $(BUILD_DIR)/sscr.bin
	-$(AC) -d $(DISK) MENU
	$(AC) -p $(DISK) MENU BIN 0x8800 < $(BUILD_DIR)/menu.bin
	-$(AC) -d $(DISK) CNSL
	$(AC) -p $(DISK) CNSL BIN 0x8800 < $(BUILD_DIR)/cnsl.bin
	-$(AC) -d $(DISK) GAME.DATA
ifeq ($(DISK_BACKEND),rwts)
	python tools/build_rwts_boot.py --root $(ROOT_DIR) --build $(BUILD_DIR) --disk $(DISK)
endif
	$(AC) -l $(DISK)

disk-rwts: prepare-rwts-disk
	"$(MAKE)" BUILD_DIR=$(ROOT_DIR)/build-rwts DISK=$(RWTS_DISK) DISK_BACKEND=rwts EXTRA_CFLAGS=-DRWTS_EXPERIMENTAL=1 disk

prepare-rwts-disk:
ifeq ($(OS),Windows_NT)
	powershell.exe -NoProfile -Command "Copy-Item -LiteralPath '$(BASE_DISK)' -Destination '$(RWTS_DISK)' -Force"
else
	cp -f $(BASE_DISK) $(RWTS_DISK)
endif

overlays: $(BUILD_DIR)/iscr.bin $(BUILD_DIR)/pscr.bin $(BUILD_DIR)/tscr.bin $(BUILD_DIR)/ascr.bin $(BUILD_DIR)/dscr.bin $(BUILD_DIR)/txac.bin $(BUILD_DIR)/bscr.bin $(BUILD_DIR)/sscr.bin $(BUILD_DIR)/menu.bin $(BUILD_DIR)/cnsl.bin

iimperialism: $(MAIN_OBJECTS) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) -o $(BUILD_DIR)/iimperialism -m $(BUILD_DIR)/iimperialism.map $(MAIN_OBJECTS)

$(OVL_CFG): iimperialism $(CONFIG_DIR)/apple2-ovl.cfg | $(BUILD_DIR)
ifeq ($(OS),Windows_NT)
	powershell.exe -NoProfile -Command "$$match = Select-String -Path '$(BUILD_DIR)/iimperialism.map' -Pattern '_state\s+([0-9A-Fa-f]{6})\s+RLA' | Select-Object -First 1; if (-not $$match) { throw 'Could not find _state in map file.' }; $$token = $$match.Matches[0].Groups[1].Value; $$state = $$token.Substring($$token.Length - 4).ToUpper(); $$content = Get-Content '$(CONFIG_DIR)/apple2-ovl.cfg'; $$content = $$content | ForEach-Object { if ($$_ -like '*_state:*') { '_state:                    type = export, value = $$' + $$state + ';' } else { $$_ } }; Set-Content '$(OVL_CFG)' $$content"
else
	@state_addr=$$(grep -Eo '_state[[:space:]]+00[0-9A-Fa-f]{4}[[:space:]]+RLA' $(BUILD_DIR)/iimperialism.map | head -n 1 | sed -E 's/.*00([0-9A-Fa-f]{4}).*/\1/'); \
	test -n "$$state_addr"; \
	sed -E "s/_state:[[:space:]]+type = export, value = \\$[0-9A-Fa-f]{4};/_state:                    type = export, value = \$$$state_addr;/" $(CONFIG_DIR)/apple2-ovl.cfg > $(OVL_CFG)
endif

$(BUILD_DIR):
ifeq ($(OS),Windows_NT)
	powershell.exe -NoProfile -Command "New-Item -ItemType Directory -Force '$(BUILD_DIR)' | Out-Null"
else
	$(MKDIR_P) $(BUILD_DIR)
endif

# Main and overlay object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD_DIR)/%.o: $(ASM_DIR)/%.s | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ -c $<

# Overlay binaries: raw 2KB images, zero-padded to exactly 2048 bytes.
# Must be on the disk alongside the main binary (DOS 3.3 type B).
# File names: ISCR, PSCR, TSCR  (use 'make disk' to update iimperialism.dsk)
$(BUILD_DIR)/iscr.bin: $(BUILD_DIR)/ovl_industry_entry.o $(BUILD_DIR)/ovl_industry.o $(OVL_CFG) | $(BUILD_DIR)
	$(OVL_CC) $(OVL_LDFLAGS) -o $(BUILD_DIR)/iscr.bin $(BUILD_DIR)/ovl_industry_entry.o $(BUILD_DIR)/ovl_industry.o

$(BUILD_DIR)/pscr.bin: $(BUILD_DIR)/ovl_production_entry.o $(BUILD_DIR)/ovl_production.o $(OVL_CFG) | $(BUILD_DIR)
	$(OVL_CC) $(OVL_LDFLAGS) -o $(BUILD_DIR)/pscr.bin $(BUILD_DIR)/ovl_production_entry.o $(BUILD_DIR)/ovl_production.o

$(BUILD_DIR)/tscr.bin: $(BUILD_DIR)/ovl_transport_entry.o $(BUILD_DIR)/ovl_transport.o $(OVL_CFG) | $(BUILD_DIR)
	$(OVL_CC) $(OVL_LDFLAGS) -o $(BUILD_DIR)/tscr.bin $(BUILD_DIR)/ovl_transport_entry.o $(BUILD_DIR)/ovl_transport.o

$(BUILD_DIR)/ascr.bin: $(BUILD_DIR)/ovl_admiralty.o $(OVL_CFG) | $(BUILD_DIR)
	$(OVL_CC) $(OVL_LDFLAGS) -o $(BUILD_DIR)/ascr.bin $(BUILD_DIR)/ovl_admiralty.o

$(BUILD_DIR)/dscr.bin: $(BUILD_DIR)/ovl_diplomacy_entry.o $(BUILD_DIR)/ovl_diplomacy.o $(OVL_CFG) | $(BUILD_DIR)
	$(OVL_CC) $(OVL_LDFLAGS) -o $(BUILD_DIR)/dscr.bin $(BUILD_DIR)/ovl_diplomacy_entry.o $(BUILD_DIR)/ovl_diplomacy.o

$(BUILD_DIR)/txac.bin: $(BUILD_DIR)/ovl_trade_expedition_action_entry.o $(BUILD_DIR)/ovl_trade_expedition_action.o $(OVL_CFG) | $(BUILD_DIR)
	$(OVL_CC) $(OVL_LDFLAGS) -o $(BUILD_DIR)/txac.bin $(BUILD_DIR)/ovl_trade_expedition_action_entry.o $(BUILD_DIR)/ovl_trade_expedition_action.o

$(BUILD_DIR)/bscr.bin: $(BUILD_DIR)/ovl_battle_entry.o $(BUILD_DIR)/ovl_battle.o $(OVL_CFG) | $(BUILD_DIR)
	$(OVL_CC) $(OVL_LDFLAGS) -o $(BUILD_DIR)/bscr.bin $(BUILD_DIR)/ovl_battle_entry.o $(BUILD_DIR)/ovl_battle.o

$(BUILD_DIR)/sscr.bin: $(BUILD_DIR)/ovl_science_entry.o $(BUILD_DIR)/ovl_science.o $(OVL_CFG) | $(BUILD_DIR)
	$(OVL_CC) $(OVL_LDFLAGS) -o $(BUILD_DIR)/sscr.bin $(BUILD_DIR)/ovl_science_entry.o $(BUILD_DIR)/ovl_science.o

$(BUILD_DIR)/menu.bin: $(BUILD_DIR)/ovl_game_menu_entry.o $(BUILD_DIR)/ovl_game_menu.o $(BUILD_DIR)/disk_gamestate_io_$(DISK_BACKEND).o $(OVL_CFG) | $(BUILD_DIR)
	$(OVL_CC) $(OVL_LDFLAGS) -o $(BUILD_DIR)/menu.bin $(BUILD_DIR)/ovl_game_menu_entry.o $(BUILD_DIR)/ovl_game_menu.o $(BUILD_DIR)/disk_gamestate_io_$(DISK_BACKEND).o

$(BUILD_DIR)/cnsl.bin: $(BUILD_DIR)/ovl_council_nations_entry.o $(BUILD_DIR)/ovl_council_nations.o $(OVL_CFG) | $(BUILD_DIR)
	$(OVL_CC) $(OVL_LDFLAGS) -o $(BUILD_DIR)/cnsl.bin $(BUILD_DIR)/ovl_council_nations_entry.o $(BUILD_DIR)/ovl_council_nations.o

$(BUILD_DIR)/ovl_diplomacy_entry.o: $(ASM_DIR)/ovl_diplomacy_entry.s | $(BUILD_DIR)
	ca65 $(ASM_DIR)/ovl_diplomacy_entry.s -o $(BUILD_DIR)/ovl_diplomacy_entry.o

$(BUILD_DIR)/ovl_industry_entry.o: $(ASM_DIR)/ovl_industry_entry.s | $(BUILD_DIR)
	ca65 $(ASM_DIR)/ovl_industry_entry.s -o $(BUILD_DIR)/ovl_industry_entry.o

$(BUILD_DIR)/ovl_production_entry.o: $(ASM_DIR)/ovl_production_entry.s | $(BUILD_DIR)
	ca65 $(ASM_DIR)/ovl_production_entry.s -o $(BUILD_DIR)/ovl_production_entry.o

$(BUILD_DIR)/ovl_trade_expedition_action_entry.o: $(ASM_DIR)/ovl_trade_expedition_action_entry.s | $(BUILD_DIR)
	ca65 $(ASM_DIR)/ovl_trade_expedition_action_entry.s -o $(BUILD_DIR)/ovl_trade_expedition_action_entry.o

$(BUILD_DIR)/ovl_transport_entry.o: $(ASM_DIR)/ovl_transport_entry.s | $(BUILD_DIR)
	ca65 $(ASM_DIR)/ovl_transport_entry.s -o $(BUILD_DIR)/ovl_transport_entry.o

$(BUILD_DIR)/ovl_battle_entry.o: $(ASM_DIR)/ovl_battle_entry.s | $(BUILD_DIR)
	ca65 $(ASM_DIR)/ovl_battle_entry.s -o $(BUILD_DIR)/ovl_battle_entry.o

$(BUILD_DIR)/ovl_science_entry.o: $(ASM_DIR)/ovl_science_entry.s | $(BUILD_DIR)
	ca65 $(ASM_DIR)/ovl_science_entry.s -o $(BUILD_DIR)/ovl_science_entry.o

$(BUILD_DIR)/ovl_game_menu_entry.o: $(ASM_DIR)/ovl_game_menu_entry.s | $(BUILD_DIR)
	ca65 $(ASM_DIR)/ovl_game_menu_entry.s -o $(BUILD_DIR)/ovl_game_menu_entry.o

$(BUILD_DIR)/ovl_council_nations_entry.o: $(ASM_DIR)/ovl_council_nations_entry.s | $(BUILD_DIR)
	ca65 $(ASM_DIR)/ovl_council_nations_entry.s -o $(BUILD_DIR)/ovl_council_nations_entry.o

$(BUILD_DIR)/loader.o: $(LOADER_DIR)/loader.s | $(BUILD_DIR)
	ca65 $(LOADER_DIR)/loader.s -o $(BUILD_DIR)/loader.o

$(BUILD_DIR)/loader.system: $(BUILD_DIR)/loader.o $(LOADER_DIR)/loader.cfg | $(BUILD_DIR)
	ld65 -C $(LOADER_DIR)/loader.cfg -o $(BUILD_DIR)/loader.system $(BUILD_DIR)/loader.o

clean:
	$(CLEAN_CMD)

memory-usage: SHELL := cmd.exe
memory-usage: iimperialism overlays
	powershell.exe -NoProfile -ExecutionPolicy Bypass -File "$(TOOLS_DIR)/memory-usage.ps1" "$(BUILD_DIR)"
