#ifndef _GRAPHIC_H
#define _GRAPHIC_H

#include "common.h"
#include "memory.h"

typedef struct gbc_graphic gbc_graphic_t;

#define VRAM_BANK_SIZE (VRAM_END-VRAM_BEGIN+1)
#define OAM_SIZE (OAM_END-OAM_BEGIN+1)

/* 
https://gbdev.io/pandocs/Rendering.html
I implemented a simplified model of the GameBoy PPU.
Graphic cycle runs every 80dots, every scanline costs 6 graphic cycles (480 dots)
The first cycle is mode 2, the next 3 cycles are in mode 3, the rests are in mode 0.
Every mode has a fixed inverval, unlike the real GameBoy.

TODO: Using cpu cycles may be better?
*/

#define FRAME_RATE 60          /* The real GameBoy runs at 59.7fps */
#define FRAME_TIME (1.0 / FRAME_RATE)
#define DOTS_PER_FRAME 70224
#define DOT_TIME (1000000000 / DOTS_PER_FRAME / FRAME_RATE)

#define DOTS_PER_SCANLINE 480       /* The real GameBoy has 456dots per scanline */
#define DOTS_PER_CYCLE 80           /* graphic updates every 80dots */
#define GRAPHIC_UPDATE_TIME (DOT_TIME * DOTS_PER_CYCLE)

#define CYCLE_MODEL_2_START    0
#define CYCLE_MODEL_3_START    1
#define CYCLE_MODEL_0_START    4
#define CYCLES_PER_SCANLINE         (DOTS_PER_SCANLINE / DOTS_PER_CYCLE)

#define TOTAL_SCANLINES 153
#define VISIBLE_SCANLINES 143

#define PPU_MODE_0 0   /* H-BLANK */
#define PPU_MODE_1 1   /* V-BLANK */
#define PPU_MODE_2 2   /* OAM SCAN */
#define PPU_MODE_3 3   /* DRAWING */

#define LCDC_PPU_ENABLE 0x80
#define LCDC_WINDOW_TILE_MAP 0x40
#define LCDC_WINDOW_ENABLE 0x20
#define LCDC_BG_WINDOW_TILE_DATA 0x10
#define LCDC_BG_TILE_MAP 0x08
#define LCDC_OBJ_SIZE 0x04
#define LCDC_OBJ_ENABLE 0x02
#define LCDC_BG_ENABLE 0x01

struct gbc_graphic
{
    uint8_t vram[VRAM_BANK_SIZE * 2]; /* 2x8KB */
    uint8_t oam[OAM_SIZE];
    uint8_t scanline;
    uint8_t scanline_cycles;
    uint8_t mode;
    
    void *screen_udata;
    void (*screen_update)(void *udata);
    memory_write screen_write;

    gbc_memory_t *mem;

    uint64_t t_delta;      /* nanoseconds */
};


void gbc_graphic_connect(gbc_graphic_t *graphic, gbc_memory_t *mem);
void gbc_graphic_init(gbc_graphic_t *graphic);
void gbc_graphic_cycle(gbc_graphic_t *graphic, uint64_t delta);

#endif