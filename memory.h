#ifndef _MEMORY_H
#define _MEMORY_H

#include "common.h"

#define WRAM_BANK_SIZE 0x1000 /* 4KB */
#define WRAM_BANKS     8

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

#define IO_PORT_ID 10
#define IO_PORT_BEGIN 0xff00
#define IO_PORT_END 0xff7f

#define HRAM_ID 11
#define HRAM_BEGIN 0xff80
#define HRAM_END 0xfffe

#define IE_REGISTER_ID 12
#define IE_REGISTER_BEGIN 0xffff
#define IE_REGISTER_END 0xffff

#define MEMORY_MAP_ENTRIES 12

#define RAM_ADDR_MASK 0x1fff   /* 13-bits 8KB */
#define RAM_ADDR_MASK_SHIFT 13

#define ROM_ADDR_MASK 0x3fff   /* 14-bits 16KB */
#define ROM_ADDR_MASK_SHIFT 14

/* IO Ports */
#define IO_PORT_BASE IO_PORT_BEGIN
#define IO_PORT_P1   0x00
#define IO_PORT_SB   0x01
#define IO_PORT_SC   0x02
#define IO_PORT_DIV  0x04
#define IO_PORT_TIMA 0x05
#define IO_PORT_TMA  0x06
#define IO_PORT_TAC  0x07
#define IO_PORT_IF   0x0f
#define IO_PORT_NR10 0x10
#define IO_PORT_NR11 0x11
#define IO_PORT_NR12 0x12
#define IO_PORT_NR13 0x13
#define IO_PORT_NR14 0x14
#define IO_PORT_NR21 0x16
#define IO_PORT_NR22 0x17
#define IO_PORT_NR23 0x18
#define IO_PORT_NR24 0x19
#define IO_PORT_NR30 0x1a
#define IO_PORT_NR31 0x1b
#define IO_PORT_NR32 0x1c
#define IO_PORT_NR33 0x1d
#define IO_PORT_NR34 0x1e
#define IO_PORT_NR41 0x20
#define IO_PORT_NR42 0x21
#define IO_PORT_NR43 0x22
#define IO_PORT_NR44 0x23
#define IO_PORT_NR50 0x24
#define IO_PORT_NR51 0x25
#define IO_PORT_NR52 0x26
#define IO_PORT_WAVE_RAM 0x30 /* 0x30 - 0x3f */
#define IO_PORT_LCDC 0x40
#define IO_PORT_STAT 0x41
#define IO_PORT_SCY  0x42
#define IO_PORT_SCX  0x43
#define IO_PORT_LY   0x44
#define IO_PORT_LYC  0x45
#define IO_PORT_DMA  0x46
#define IO_PORT_BGP  0x47
#define IO_PORT_OBP0 0x48
#define IO_PORT_OBP1 0x49
#define IO_PORT_WY   0x4a
#define IO_PORT_WX   0x4b
#define IO_PORT_KEY1 0x4d
#define IO_PORT_VBK  0x4f
#define IO_PORT_HDMA1 0x51
#define IO_PORT_HDMA2 0x52
#define IO_PORT_HDMA3 0x53
#define IO_PORT_HDMA4 0x54
#define IO_PORT_HDMA5 0x55
#define IO_PORT_RP   0x56    
#define IO_PORT_BCPS_BCPI 0x68
#define IO_PORT_BCPD_BGPD 0x69
#define IO_PORT_OCPS_OCPI 0x6a
#define IO_PORT_OCPD_OBPD 0x6b
#define IO_PORT_OPRI 0x6c
#define IO_PORT_SVBK 0x70
#define IO_PORT_PCM12 0x76
#define IO_PORT_PCM34 0x77

#define IO_ADDR_PORT(addr) ((addr) - IO_PORT_BASE)
#define IO_PORT_ADDR(port) ((port) + IO_PORT_BASE)

#define IO_PORT_READ(mem, port) ((mem)->io_ports[(port)])
#define IO_PORT_WRITE(mem, port, data) ((mem)->io_ports[(port)] = (data))

#define REQUEST_INTERRUPT(mem, intp) ((mem)->io_ports[IO_PORT_IF] |= (intp))

#define BG_PALETTE_READ(mem, idx) ((mem)->bg_palette + ((idx)))
#define OBJ_PALETTE_READ(mem, idx) ((mem)->obj_palette + ((idx)))

#define OAM_ADDR(mem) ((mem)->oam)

typedef struct gbc_memory gbc_memory_t;
typedef struct memory_map_entry memory_map_entry_t;
typedef struct gbc_palette gbc_palette_t;

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

struct gbc_palette
{
    uint16_t c[4]; /* 4 colors x 2 bytes per color */
};

/* It is actually, bus */
struct gbc_memory
{
    memory_read read;
    memory_write write;
    memory_map_entry_t map[MEMORY_MAP_ENTRIES];
    uint8_t wram[WRAM_BANK_SIZE * WRAM_BANKS];
    uint8_t hraw[HRAM_END - HRAM_BEGIN + 1];
    uint8_t io_ports[IO_PORT_END - IO_PORT_BEGIN + 1]; 
    uint8_t oam[OAM_END - OAM_BEGIN + 1];
    /* https://gbdev.io/pandocs/Palettes.html#lcd-color-palettes-cgb-only */
    /* palatte memory */
    gbc_palette_t bg_palette[8];
    gbc_palette_t obj_palette[8];    
};

void gbc_mem_init(gbc_memory_t *mem);
void register_memory_map(gbc_memory_t *mem, memory_map_entry_t *entry);
void* connect_io_port(gbc_memory_t *mem, uint16_t addr);

#endif 
