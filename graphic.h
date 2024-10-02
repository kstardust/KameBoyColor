#ifndef _GRAPHIC_H
#define _GRAPHIC_H

#include "common.h"
#include "memory.h"

typedef struct gbc_graphic gbc_graphic_t;
typedef struct gbc_tile gbc_tile_t;
typedef struct gbc_tilemap gbc_tilemap_t;
typedef struct gbc_tilemap_attr gbc_tilemap_attr_t;
typedef struct gbc_obj gbc_obj_t;

typedef void (*screen_write)(void *udata, uint16_t addr, uint16_t data);

#define VRAM_BANK_SIZE (VRAM_END-VRAM_BEGIN+1)

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
#define DOTS_PER_FRAME 73920
#define DOT_TIME (1000000000 / DOTS_PER_FRAME / FRAME_RATE)

#define TOTAL_SCANLINES 153      /* 0 - 153 */
#define VISIBLE_SCANLINES 143    /* 0 - 143 */

#define VISIBLE_HORIZONTAL_PIXELS 160
#define VISIBLE_VERTICAL_PIXELS 144

#define DOTS_PER_SCANLINE 456
#define PPU_MODE_2_DOTS 80
#define PPU_MODE_0_DOTS 100 /* 87 ~ 204, i randomly picked 100 */
#define PPU_MODE_3_DOTS (DOTS_PER_SCANLINE - PPU_MODE_0_DOTS - PPU_MODE_2_DOTS)
#define PPU_MODE_1_DOTS DOTS_PER_SCANLINE

#define DOTS_MODEL_2_START   0
#define DOTS_MODEL_3_START    (PPU_MODE_2_DOTS)
#define DOTS_MODEL_0_START    (PPU_MODE_2_DOTS + PPU_MODE_3_DOTS)

#define PPU_MODE_0 0   /* H-BLANK */
#define PPU_MODE_1 1   /* V-BLANK */
#define PPU_MODE_2 2   /* OAM SCAN */
#define PPU_MODE_3 3   /* DRAWING */
#define PPU_MODE_MASK 0x03

#define LCDC_PPU_ENABLE 0x80
#define LCDC_WINDOW_TILE_MAP 0x40
#define LCDC_WINDOW_ENABLE 0x20
#define LCDC_BG_WINDOW_TILE_DATA 0x10
#define LCDC_BG_TILE_MAP 0x08
#define LCDC_OBJ_SIZE 0x04
#define LCDC_OBJ_ENABLE 0x02
#define LCDC_BG_ENABLE 0x01

#define STAT_LYC_LY 0x04
#define STAT_MODE_0_INT 0x08
#define STAT_MODE_1_INT 0x10
#define STAT_MODE_2_INT 0x20
#define STAT_LYC_INT 0x40

#define TILE_SIZE 8        /* each tile is 8x8 pixels */
#define TILE_MAP_SIZE 256  /* 32x32 tiles */

#define OBJ_WIDTH 8
#define OBJ_HEIGHT 8
#define OBJ_HEIGHT_2 16

#define TILE_TYPE_OBJ  1
#define TILE_TYPE_BG   2
#define TILE_TYPE_WIN  3

#define TILE_ATTR_PALETTE(x) ((x) & 0x07)
#define TILE_ATTR_VRAM_BANK(x) ((x) & 0x08)
#define TILE_ATTR_XFLIP(x) ((x) & 0x20)
#define TILE_ATTR_YFLIP(x) ((x) & 0x40)
#define TILE_ATTR_PRIORITY(x) ((x) & 0x80)


#define GBC_COLOR_TO_RGB_R(x) ((x & 0x1F) * 0xff / 0x1F)
#define GBC_COLOR_TO_RGB_G(x) (((x & 0x03E0) >> 5) * 0xff / 0x1F)
#define GBC_COLOR_TO_RGB_B(x) (((x & 0x7C00) >> 10) * 0xff / 0x1F)

#define GBC_COLOR_TO_RGB(x) (GBC_COLOR_TO_RGB_R(x) << 10 | GBC_COLOR_TO_RGB_G(x) << 5 | GBC_COLOR_TO_RGB_B(x))

#define MAX_OBJ_SCANLINE 10
#define MAX_OBJS ((OAM_END - OAM_BEGIN + 1) / 4)

#define OBJ_ATTR_PALETTE(x) ((x) & 0x07)
#define OBJ_ATTR_VRAM_BANK(x) ((x) & 0x08)
#define OBJ_ATTR_XFLIP(x) ((x) & 0x20)
#define OBJ_ATTR_YFLIP(x) ((x) & 0x40)
#define OBJ_ATTR_BG_PRIORITY(x) ((x) & 0x80)

#define OAM_Y_TO_SCREEN(y) ((y) - 16)
#define OAM_X_TO_SCREEN(x) ((x) - 8)

struct gbc_graphic
{
    uint32_t dots;   /* dots to next graphic update */
    uint8_t vram[VRAM_BANK_SIZE * 2]; /* 2x8KB */
    uint8_t scanline;
    uint8_t mode;

    void *screen_udata;
    void (*screen_update)(void *udata);
    screen_write screen_write;

    gbc_memory_t *mem;
};

struct gbc_tile
{
    uint8_t data[16];
};

struct gbc_tilemap
{
    /* 32 x 32 tiles */
    uint8_t data[32][32];
};

struct gbc_tilemap_attr
{
    /* 32 x 32 tiles */
    uint8_t data[32][32];
};

struct gbc_obj
{
    uint8_t y;
    uint8_t x;
    uint8_t tile;
    uint8_t attr;
};


void gbc_graphic_connect(gbc_graphic_t *graphic, gbc_memory_t *mem);
void gbc_graphic_init(gbc_graphic_t *graphic);
void gbc_graphic_cycle(gbc_graphic_t *graphic);
uint8_t* gbc_graphic_get_tile_attr(gbc_graphic_t *graphic, uint8_t type, uint8_t idx);
gbc_tile_t* gbc_graphic_get_tile(gbc_graphic_t *graphic, uint8_t type, uint8_t idx, uint8_t bank);


#endif