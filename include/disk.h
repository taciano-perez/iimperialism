#ifndef DISK_H
#define DISK_H

#include "game.h"

#define DISK_ERR_NONE 0U
#define DISK_ERR_BACKEND_NOT_READY 254U
#define DISK_ERR_INVALID_OVERLAY 255U

#define DISK_CAP_SAVE_LOAD 0x01U
#define DISK_SAVE_SLOT_COUNT 5U

typedef struct {
    unsigned char valid;
    unsigned int turn_number;
    char nation_name[20];
} DiskSaveSlotInfo;

unsigned char __fastcall__ disk_get_capabilities(void);
unsigned char __fastcall__ disk_load_overlay(unsigned char id);
extern unsigned int disk_overlay_bytes_read;

unsigned char __fastcall__ disk_save_game(const GameState* state);
unsigned char __fastcall__ disk_load_game(GameState* state);
unsigned char __fastcall__ disk_read_save_slot_info(DiskSaveSlotInfo* info);

#endif /* DISK_H */
