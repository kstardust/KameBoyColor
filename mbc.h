#ifndef _MBC_H
#define _MBC_H

#include "common.h"
#include "memory.h"
#include "cartridge.h"

#define MAX_ROM_BANKS 512
#define MAX_RAM_BANKS 16
#define ROM_BANK_SIZE 0x4000    /* 16KB */
#define RAM_BANK_SIZE 0x2000    /* 8KB */

#define MBC1_REG_RAM_ENABLE_BEGIN 0x0000
#define MBC1_REG_RAM_ENABLE_END   0x1fff

#define MBC1_REG_ROM_BANK_BEGIN 0x2000
#define MBC1_REG_ROM_BANK_END   0x3fff

#define MBC1_REG_RAM_BANK_BEGIN 0x4000
#define MBC1_REG_RAM_BANK_END   0x5fff

#define MBC1_REG_BANKING_MODE_BEGIN 0x6000
#define MBC1_REG_BANKING_MODE_END   0x7fff


typedef struct gbc_mbc gbc_mbc_t;
typedef uint8_t (*mbc_read_func)(gbc_mbc_t *mbc, uint16_t addr);
typedef uint8_t (*mbc_write_func)(gbc_mbc_t *mbc, uint16_t addr, uint8_t data);

struct gbc_mbc 
{
    uint8_t *rom_banks;
    uint8_t *ram_banks;
    uint8_t rom_bank;
    uint8_t ram_bank;

    uint8_t rom_bank_size;
    uint8_t ram_bank_size;
    uint8_t ram_enabled;
    uint8_t mode;

    uint8_t type;

    mbc_read_func read;
    mbc_write_func write;
    
    gbc_memory_t *mem;
};

gbc_mbc_t gbc_mbc_new();
void gbc_mbc_connect(gbc_mbc_t *mbc, gbc_memory_t *mem);
void gbc_mbc_init_with_cart(gbc_mbc_t *mbc, cartridge_t *cart);

#endif