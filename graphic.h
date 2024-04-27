#ifndef _GRAPHIC_H
#define _GRAPHIC_H

#include "common.h"
#include "memory.h"

typedef struct gbc_graphic gbc_graphic_t;

#define VRAM_BANK_SIZE (VRAM_END-VRAM_BEGIN+1)
#define OAM_SIZE (OAM_END-OAM_BEGIN+1)

struct gbc_graphic
{
    uint8_t vram[VRAM_BANK_SIZE * 2]; /* 2x8KB */
    uint8_t oam[OAM_SIZE];
    gbc_memory_t *mem;
};

void gbc_graphic_connect(gbc_graphic_t *graphic, gbc_memory_t *mem);
void gbc_graphic_init(gbc_graphic_t *graphic);

#endif