# Makefile for cc65 Apple II target

ROOT_DIR   = $(CURDIR)
SRC_DIR    = $(ROOT_DIR)/src
ASM_DIR    = $(ROOT_DIR)/asm
INCLUDE_DIR= $(ROOT_DIR)/include
CONFIG_DIR = $(ROOT_DIR)/config
ASSETS_DIR = $(ROOT_DIR)/assets
TOOLS_DIR  = $(ROOT_DIR)/tools
BUILD_DIR  = $(ROOT_DIR)/build

ifeq ($(OS),Windows_NT)
MKDIR_P  = mkdir -p
MV_CMD    = mv -f
CLEAN_CMD = powershell.exe -NoProfile -Command "if (Test-Path '$(BUILD_DIR)') { Remove-Item -Recurse -Force -ErrorAction SilentlyContinue '$(BUILD_DIR)' }; Remove-Item -Force -ErrorAction SilentlyContinue '$(SRC_DIR)/*.o','$(ASM_DIR)/*.o'"
else
MKDIR_P  = mkdir -p
MV_CMD    = mv -f
CLEAN_CMD = rm -rf $(BUILD_DIR) $(SRC_DIR)/*.o $(ASM_DIR)/*.o
endif

C_SOURCES  = \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/ui.c \
	$(SRC_DIR)/gamestate.c \
	$(SRC_DIR)/logic.c \
	$(SRC_DIR)/overlay.c \
	$(SRC_DIR)/ovl_industry.c \
	$(SRC_DIR)/ovl_production.c \
	$(SRC_DIR)/ovl_transport.c \
	$(SRC_DIR)/ovl_admiralty.c

ASM_SOURCES = \
	$(ASM_DIR)/werner.s \
	$(ASM_DIR)/ovl_asm.s \
	$(ASM_DIR)/jmptab.s

MAIN_OBJECTS = \
	$(BUILD_DIR)/main.o \
	$(BUILD_DIR)/ui.o \
	$(BUILD_DIR)/werner.o \
	$(BUILD_DIR)/gamestate.o \
	$(BUILD_DIR)/logic.o \
	$(BUILD_DIR)/overlay.o \
	$(BUILD_DIR)/ovl_asm.o \
	$(BUILD_DIR)/jmptab.o

OVERLAY_OBJECTS = \
	$(BUILD_DIR)/ovl_industry.o \
	$(BUILD_DIR)/ovl_production.o \
	$(BUILD_DIR)/ovl_transport.o \
	$(BUILD_DIR)/ovl_admiralty.o

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

all: $(BUILD_DIR) iimperialism overlays

# Update disk image with latest binaries (run after 'make all')
disk: iimperialism overlays
	$(AC) -d $(DISK) STARTUP        2>/dev/null; $(AC) -bas $(DISK) STARTUP      < $(ASSETS_DIR)/startup.bas
	$(AC) -d $(DISK) IIMPERIALISM   2>/dev/null; $(AC) -p   $(DISK) IIMPERIALISM BIN 0x0803 < $(BUILD_DIR)/iimperialism
	$(AC) -d $(DISK) ISCR           2>/dev/null; $(AC) -p   $(DISK) ISCR         BIN 0x8800 < $(BUILD_DIR)/iscr.bin
	$(AC) -d $(DISK) PSCR           2>/dev/null; $(AC) -p   $(DISK) PSCR         BIN 0x9000 < $(BUILD_DIR)/pscr.bin
	$(AC) -d $(DISK) TSCR           2>/dev/null; $(AC) -p   $(DISK) TSCR         BIN 0x9800 < $(BUILD_DIR)/tscr.bin
	$(AC) -d $(DISK) ASCR           2>/dev/null; $(AC) -p   $(DISK) ASCR         BIN 0xA000 < $(BUILD_DIR)/ascr.bin
	$(AC) -l $(DISK)

overlays: $(BUILD_DIR)/iscr.bin $(BUILD_DIR)/pscr.bin $(BUILD_DIR)/tscr.bin $(BUILD_DIR)/ascr.bin

iimperialism: $(MAIN_OBJECTS) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) -o $(BUILD_DIR)/iimperialism -m $(BUILD_DIR)/iimperialism.map $(MAIN_OBJECTS)

$(BUILD_DIR):
	$(MKDIR_P) $(BUILD_DIR)

# Main and overlay object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $<
	$(MV_CMD) $(SRC_DIR)/$*.o $@

$(BUILD_DIR)/%.o: $(ASM_DIR)/%.s | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $<
	$(MV_CMD) $(ASM_DIR)/$*.o $@

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

clean:
	$(CLEAN_CMD)
