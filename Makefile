# Makefile for cc65 Apple II target

ROOT_DIR   = $(CURDIR)
SRC_DIR    = $(ROOT_DIR)/src
ASM_DIR    = $(ROOT_DIR)/asm
INCLUDE_DIR= $(ROOT_DIR)/include
CONFIG_DIR = $(ROOT_DIR)/config
ASSETS_DIR = $(ROOT_DIR)/assets
TOOLS_DIR  = $(ROOT_DIR)/tools
BUILD_DIR  = $(ROOT_DIR)/build
LOADER_DIR = $(ROOT_DIR)/asm/loader

ifeq ($(OS),Windows_NT)
CLEAN_CMD = powershell.exe -NoProfile -Command "if (Test-Path '$(BUILD_DIR)') { Remove-Item -Recurse -Force -ErrorAction SilentlyContinue '$(BUILD_DIR)' }; Remove-Item -Force -ErrorAction SilentlyContinue '$(SRC_DIR)/*.o','$(ASM_DIR)/*.o'"
else
MKDIR_P  = mkdir -p
CLEAN_CMD = rm -rf $(BUILD_DIR) $(SRC_DIR)/*.o $(ASM_DIR)/*.o
endif

C_SOURCES  = \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/industry.c \
	$(SRC_DIR)/transport.c \
	$(SRC_DIR)/production.c \
	$(SRC_DIR)/admiralty.c \
	$(SRC_DIR)/strings.c \
	$(SRC_DIR)/ui.c \
	$(SRC_DIR)/gamestate.c \
	$(SRC_DIR)/random.c \
	$(SRC_DIR)/logic.c \
	$(SRC_DIR)/overlay.c \
	$(SRC_DIR)/ovl_industry.c \
	$(SRC_DIR)/ovl_production.c \
	$(SRC_DIR)/ovl_transport.c \
	$(SRC_DIR)/ovl_admiralty.c \
	$(SRC_DIR)/ovl_admiralty_trader.c \
	$(SRC_DIR)/ovl_admiralty_warship.c \
	$(SRC_DIR)/ovl_diplomacy.c \
	$(SRC_DIR)/ovl_trade_expedition.c \
	$(SRC_DIR)/ovl_trade_expedition_action.c \
	$(SRC_DIR)/ovl_battle.c

ASM_SOURCES = \
	$(ASM_DIR)/werner.s \
	$(ASM_DIR)/ovl_asm.s \
	$(ASM_DIR)/jmptab.s

MAIN_OBJECTS = \
	$(BUILD_DIR)/main.o \
	$(BUILD_DIR)/industry.o \
	$(BUILD_DIR)/transport.o \
	$(BUILD_DIR)/production.o \
	$(BUILD_DIR)/admiralty.o \
	$(BUILD_DIR)/strings.o \
	$(BUILD_DIR)/ui.o \
	$(BUILD_DIR)/werner.o \
	$(BUILD_DIR)/gamestate.o \
	$(BUILD_DIR)/random.o \
	$(BUILD_DIR)/logic.o \
	$(BUILD_DIR)/overlay.o \
	$(BUILD_DIR)/prodos_overlay_load.o \
	$(BUILD_DIR)/ovl_asm.o \
	$(BUILD_DIR)/jmptab.o

OVERLAY_OBJECTS = \
	$(BUILD_DIR)/ovl_industry.o \
	$(BUILD_DIR)/ovl_production.o \
	$(BUILD_DIR)/ovl_transport.o \
	$(BUILD_DIR)/ovl_admiralty.o \
	$(BUILD_DIR)/ovl_admiralty_trader.o \
	$(BUILD_DIR)/ovl_admiralty_warship.o \
	$(BUILD_DIR)/ovl_diplomacy.o \
	$(BUILD_DIR)/ovl_trade_expedition.o \
	$(BUILD_DIR)/ovl_trade_expedition_action.o \
	$(BUILD_DIR)/ovl_battle.o

# Main compiler

CC      = cl65
CFLAGS  = -t apple2 -Oirs -I$(INCLUDE_DIR)
LDFLAGS = -t apple2 -C $(CONFIG_DIR)/apple2-hgr.cfg -Oirs

# Overlay linker: raw 2KB binary, no startup, symbols from jump table
OVL_CC = cl65
OVL_LDFLAGS = -t apple2 -C $(CONFIG_DIR)/apple2-ovl.cfg -Oirs

# Disk image tool (AppleCommander)
AC = java -jar $(TOOLS_DIR)/ac.jar
DISK = $(ASSETS_DIR)/iimperialism.dsk
LOADER_SYSTEM = IIMP.SYSTEM

all: $(BUILD_DIR) iimperialism overlays $(BUILD_DIR)/loader.system

# Update disk image with latest binaries (run after 'make all')
disk: iimperialism overlays $(BUILD_DIR)/loader.system
	-$(AC) -d $(DISK) STARTUP
	-$(AC) -d $(DISK) BASIC.SYSTEM
	-$(AC) -d $(DISK) IIMPERIALISM.SYSTEM
	-$(AC) -d $(DISK) IIMPERIALISM.SY
	-$(AC) -d $(DISK) $(LOADER_SYSTEM)
	$(AC) -p $(DISK) $(LOADER_SYSTEM) SYS < $(BUILD_DIR)/loader.system
	-$(AC) -d $(DISK) IIMPERIALISM
	$(AC) -p $(DISK) IIMPERIALISM BIN 0x0803 < $(BUILD_DIR)/iimperialism
	-$(AC) -d $(DISK) ISCR
	$(AC) -p $(DISK) ISCR BIN 0x8800 < $(BUILD_DIR)/iscr.bin
	-$(AC) -d $(DISK) PSCR
	$(AC) -p $(DISK) PSCR BIN 0x9000 < $(BUILD_DIR)/pscr.bin
	-$(AC) -d $(DISK) TSCR
	$(AC) -p $(DISK) TSCR BIN 0x9800 < $(BUILD_DIR)/tscr.bin
	-$(AC) -d $(DISK) ASCR
	$(AC) -p $(DISK) ASCR BIN 0xA000 < $(BUILD_DIR)/ascr.bin
	-$(AC) -d $(DISK) ATRD
	$(AC) -p $(DISK) ATRD BIN 0x8800 < $(BUILD_DIR)/atrd.bin
	-$(AC) -d $(DISK) AWRS
	$(AC) -p $(DISK) AWRS BIN 0x8800 < $(BUILD_DIR)/awrs.bin
	-$(AC) -d $(DISK) DSCR
	$(AC) -p $(DISK) DSCR BIN 0x8800 < $(BUILD_DIR)/dscr.bin
	-$(AC) -d $(DISK) TEXP
	$(AC) -p $(DISK) TEXP BIN 0x8800 < $(BUILD_DIR)/texp.bin
	-$(AC) -d $(DISK) TXAC
	$(AC) -p $(DISK) TXAC BIN 0x8800 < $(BUILD_DIR)/txac.bin
	-$(AC) -d $(DISK) BSCR
	$(AC) -p $(DISK) BSCR BIN 0x8800 < $(BUILD_DIR)/bscr.bin
	$(AC) -l $(DISK)

overlays: $(BUILD_DIR)/iscr.bin $(BUILD_DIR)/pscr.bin $(BUILD_DIR)/tscr.bin $(BUILD_DIR)/ascr.bin $(BUILD_DIR)/atrd.bin $(BUILD_DIR)/awrs.bin $(BUILD_DIR)/dscr.bin $(BUILD_DIR)/texp.bin $(BUILD_DIR)/txac.bin $(BUILD_DIR)/bscr.bin

iimperialism: $(MAIN_OBJECTS) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) -o $(BUILD_DIR)/iimperialism -m $(BUILD_DIR)/iimperialism.map $(MAIN_OBJECTS)

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
$(BUILD_DIR)/iscr.bin: $(BUILD_DIR)/ovl_industry.o | $(BUILD_DIR)
	$(OVL_CC) $(OVL_LDFLAGS) -o $(BUILD_DIR)/iscr.bin $(BUILD_DIR)/ovl_industry.o

$(BUILD_DIR)/pscr.bin: $(BUILD_DIR)/ovl_production.o | $(BUILD_DIR)
	$(OVL_CC) $(OVL_LDFLAGS) -o $(BUILD_DIR)/pscr.bin $(BUILD_DIR)/ovl_production.o

$(BUILD_DIR)/tscr.bin: $(BUILD_DIR)/ovl_transport.o | $(BUILD_DIR)
	$(OVL_CC) $(OVL_LDFLAGS) -o $(BUILD_DIR)/tscr.bin $(BUILD_DIR)/ovl_transport.o

$(BUILD_DIR)/ascr.bin: $(BUILD_DIR)/ovl_admiralty.o | $(BUILD_DIR)
	$(OVL_CC) $(OVL_LDFLAGS) -o $(BUILD_DIR)/ascr.bin $(BUILD_DIR)/ovl_admiralty.o

$(BUILD_DIR)/atrd.bin: $(BUILD_DIR)/ovl_admiralty_trader.o | $(BUILD_DIR)
	$(OVL_CC) $(OVL_LDFLAGS) -o $(BUILD_DIR)/atrd.bin $(BUILD_DIR)/ovl_admiralty_trader.o

$(BUILD_DIR)/awrs.bin: $(BUILD_DIR)/ovl_admiralty_warship.o | $(BUILD_DIR)
	$(OVL_CC) $(OVL_LDFLAGS) -o $(BUILD_DIR)/awrs.bin $(BUILD_DIR)/ovl_admiralty_warship.o

$(BUILD_DIR)/dscr.bin: $(BUILD_DIR)/ovl_diplomacy_entry.o $(BUILD_DIR)/ovl_diplomacy.o | $(BUILD_DIR)
	$(OVL_CC) $(OVL_LDFLAGS) -o $(BUILD_DIR)/dscr.bin $(BUILD_DIR)/ovl_diplomacy_entry.o $(BUILD_DIR)/ovl_diplomacy.o

$(BUILD_DIR)/texp.bin: $(BUILD_DIR)/ovl_trade_expedition_entry.o $(BUILD_DIR)/ovl_trade_expedition.o | $(BUILD_DIR)
	$(OVL_CC) $(OVL_LDFLAGS) -o $(BUILD_DIR)/texp.bin $(BUILD_DIR)/ovl_trade_expedition_entry.o $(BUILD_DIR)/ovl_trade_expedition.o

$(BUILD_DIR)/txac.bin: $(BUILD_DIR)/ovl_trade_expedition_action_entry.o $(BUILD_DIR)/ovl_trade_expedition_action.o | $(BUILD_DIR)
	$(OVL_CC) $(OVL_LDFLAGS) -o $(BUILD_DIR)/txac.bin $(BUILD_DIR)/ovl_trade_expedition_action_entry.o $(BUILD_DIR)/ovl_trade_expedition_action.o

$(BUILD_DIR)/bscr.bin: $(BUILD_DIR)/ovl_battle.o | $(BUILD_DIR)
	$(OVL_CC) $(OVL_LDFLAGS) -o $(BUILD_DIR)/bscr.bin $(BUILD_DIR)/ovl_battle.o

$(BUILD_DIR)/ovl_diplomacy_entry.o: $(ASM_DIR)/ovl_diplomacy_entry.s | $(BUILD_DIR)
	ca65 $(ASM_DIR)/ovl_diplomacy_entry.s -o $(BUILD_DIR)/ovl_diplomacy_entry.o

$(BUILD_DIR)/ovl_trade_expedition_entry.o: $(ASM_DIR)/ovl_trade_expedition_entry.s | $(BUILD_DIR)
	ca65 $(ASM_DIR)/ovl_trade_expedition_entry.s -o $(BUILD_DIR)/ovl_trade_expedition_entry.o

$(BUILD_DIR)/ovl_trade_expedition_action_entry.o: $(ASM_DIR)/ovl_trade_expedition_action_entry.s | $(BUILD_DIR)
	ca65 $(ASM_DIR)/ovl_trade_expedition_action_entry.s -o $(BUILD_DIR)/ovl_trade_expedition_action_entry.o

$(BUILD_DIR)/loader.o: $(LOADER_DIR)/loader.s | $(BUILD_DIR)
	ca65 $(LOADER_DIR)/loader.s -o $(BUILD_DIR)/loader.o

$(BUILD_DIR)/loader.system: $(BUILD_DIR)/loader.o $(LOADER_DIR)/loader.cfg | $(BUILD_DIR)
	ld65 -C $(LOADER_DIR)/loader.cfg -o $(BUILD_DIR)/loader.system $(BUILD_DIR)/loader.o

clean:
	$(CLEAN_CMD)

memory-usage: SHELL := cmd.exe
memory-usage: iimperialism overlays
	powershell.exe -NoProfile -ExecutionPolicy Bypass -File "$(TOOLS_DIR)/memory-usage.ps1" "$(BUILD_DIR)"
