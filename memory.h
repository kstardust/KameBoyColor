#ifndef _MEMORY_H
#define _MEMORY_H

#include "common.h"

#define WRAM_BANK_SIZE 0x1000 /* 4KB */
#define WRAM_BANKS     8

#define MEMORY_MAP_ENTRIES 12

#define ROM_BANK_0_ID 1
#define ROM_BANK_0_BEGIN 0x0000
#define ROM_BANK_0_END 0x3fff

#define ROM_BANK_N_ID 2
#define ROM_BANK_N_BEGIN 0x4000
#define ROM_BANK_N_END 0x7fff

#define VRAM_ID 3
#define VRAM_BEGIN 0x8000
#define VRAM_END 0x9fff

#define EXRAM_ID 4
#define EXRAM_BEGIN 0xa000
#define EXRAM_END 0xbfff

#define WRAM_BANK_0_ID 5
#define WRAM_BANK_0_BEGIN 0xc000
#define WRAM_BANK_0_END 0xcfff

#define WRAM_BANK_N_ID 6
#define WRAM_BANK_N_BEGIN 0xd000
#define WRAM_BANK_N_END 0xdfff

#define WRAM_ECHO_ID 7
#define WRAM_ECHO_BEGIN 0xe000
#define WRAM_ECHO_END 0xfdff

#define OAM_ID 8
#define OAM_BEGIN 0xfe00
#define OAM_END 0xfe9f

#define IO_NOT_USABLE_ID 9
#define IO_NOT_USABLE_BEGIN 0xfea0
#define IO_NOT_USABLE_END 0xfeff

#define IO_ID 10
#define IO_BEGIN 0xff00
#define IO_END 0xff7f

#define HRAM_ID 11
#define HRAM_BEGIN 0xff80
#define HRAM_END 0xfffe

#define IE_REGISTER_ID 12
#define IE_REGISTER_BEGIN 0xffff
#define IE_REGISTER_END 0xffff

#define RAM_ADDR_MASK 0x1fff   /* 13-bits 8KB */
#define RAM_ADDR_MASK_SHIFT 13

#define ROM_ADDR_MASK 0x3fff   /* 14-bits 16KB */
#define ROM_ADDR_MASK_SHIFT 14


typedef struct gbc_memory gbc_memory_t;
typedef struct memory_bank memory_bank_t;
typedef struct memory_map_entry memory_map_entry_t;

typedef uint8_t (*memory_read)(void *udata, uint16_t addr);
typedef uint8_t (*memory_write)(void *udata, uint16_t addr, uint8_t data);

struct memory_map_entry
{        
    uint16_t id;
    uint16_t addr_begin;
    uint16_t addr_end;
    memory_read read;
    memory_write write;
    void *udata;
};

/* It is actually, bus */
struct gbc_memory
{
    memory_read read;
    memory_write write;    
    memory_map_entry_t map[MEMORY_MAP_ENTRIES];
    uint8_t wram[WRAM_BANKS * WRAM_BANKS];
};

struct memory_bank
{    
    uint8_t id;
    uint16_t size;
    memory_read read;
    memory_write write;
    uint8_t *data;
};

void gbc_mem_init(gbc_memory_t *mem);
void register_memory_map(gbc_memory_t *mem, memory_map_entry_t *entry);

#endif 
