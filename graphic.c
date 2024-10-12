#include "graphic.h"
#include "memory.h"
#include "cpu.h"

static void* vram_addr(void *udata, uint16_t addr);
static void* vram_addr_bank(void *udata, uint16_t addr, uint8_t bank);

void
gbc_graphic_init(gbc_graphic_t *graphic)
{
    memset(graphic, 0, sizeof(gbc_graphic_t));
}

gbc_tile_t*
gbc_graphic_get_tile(gbc_graphic_t *graphic, uint8_t type, uint8_t idx, uint8_t bank)
{
    if (type == TILE_TYPE_OBJ ||
        (IO_PORT_READ(graphic->mem, IO_PORT_LCDC) & LCDC_BG_WINDOW_TILE_DATA)) {
        return (gbc_tile_t*)(vram_addr_bank(graphic, 0x8000, bank) + idx * 16);
    }
    return (gbc_tile_t*)vram_addr_bank(graphic, 0x9000 + (int8_t)idx * 16, bank);
}

gbc_tilemap_attr_t*
gbc_graphic_get_tilemap_attr(gbc_graphic_t *graphic, uint8_t type)
{
    uint8_t lcdc = IO_PORT_READ(graphic->mem, IO_PORT_LCDC);
    uint16_t addr = 0;

    if (type == TILE_TYPE_BG) {
        addr = lcdc & LCDC_BG_TILE_MAP ? 0x9C00 : 0x9800;
    } else if (type == TILE_TYPE_WIN) {
        addr = lcdc & LCDC_WINDOW_TILE_MAP ? 0x9C00 : 0x9800;
    } else {
        LOG_ERROR("Invalid tile type\n");
        assert(0);
    }
    /* BG attr tilemap  is always in bank 1 */
    return (gbc_tilemap_attr_t*)vram_addr_bank(graphic, addr, 1);
}

gbc_tilemap_t*
gbc_graphic_get_tilemap(gbc_graphic_t *graphic, uint8_t type)
{
    uint8_t lcdc = IO_PORT_READ(graphic->mem, IO_PORT_LCDC);
    uint16_t addr = 0;

    if (type == TILE_TYPE_BG) {
        addr = lcdc & LCDC_BG_TILE_MAP ? 0x9C00 : 0x9800;
    } else if (type == TILE_TYPE_WIN) {
        addr = lcdc & LCDC_WINDOW_TILE_MAP ? 0x9C00 : 0x9800;
    } else {
        LOG_ERROR("Invalid tile type\n");
        assert(0);
    }

    /* BG tilemap is always in bank 0 */
    return (gbc_tilemap_t*)vram_addr_bank(graphic, addr, 0);
}

inline static uint16_t
gbc_graphic_render_pixel(gbc_graphic_t *graphic, uint16_t scanline, int16_t col, uint8_t *objs_idx, uint8_t objs_count)
{
    uint8_t lcdc = IO_PORT_READ(graphic->mem, IO_PORT_LCDC);
    uint16_t bg_color, obj_color;
    uint16_t tile_x, tile_y, x, y, tile_x_offset, tile_y_offset;
    uint8_t attr, bg_color_id;

    gbc_palette_t *palette;
    gbc_tile_t *tile;
    uint8_t bgwin_priority = 0;
    uint8_t lcdc_bit0 = lcdc & LCDC_BG_ENABLE;

    bg_color = obj_color = 0;
    bg_color_id = 0;

    if (lcdc_bit0) {
        /* background */
        /* TODO:
        "The scroll registers are re-read on each tile fetch, except for the low 3 bits of SCX" Does it matter?
        https://gbdev.io/pandocs/Scrolling.html#mid-frame-behavior
        */
        uint16_t scroll_x = IO_PORT_READ(graphic->mem, IO_PORT_SCX);
        uint16_t scroll_y = IO_PORT_READ(graphic->mem, IO_PORT_SCY);
        gbc_tilemap_t *bg_tilemap = gbc_graphic_get_tilemap(graphic, TILE_TYPE_BG);
        gbc_tilemap_attr_t *bg_tilemap_attr = gbc_graphic_get_tilemap_attr(graphic, TILE_TYPE_BG);

        x = (scroll_x + col) % TILE_MAP_SIZE;
        y = (scroll_y + scanline) % TILE_MAP_SIZE;

        tile_x = x / TILE_SIZE;
        tile_y = y / TILE_SIZE;

        tile_x_offset = x % TILE_SIZE;
        tile_y_offset = y % TILE_SIZE;

        attr = bg_tilemap_attr->data[tile_y][tile_x];
        tile = gbc_graphic_get_tile(graphic, TILE_TYPE_BG, bg_tilemap->data[tile_y][tile_x],
                TILE_ATTR_VRAM_BANK(attr) ? 1 : 0);

        if (TILE_ATTR_XFLIP(attr)) {
            tile_x_offset = TILE_SIZE - tile_x_offset - 1;
        }
        if (TILE_ATTR_YFLIP(attr)) {
            tile_y_offset = TILE_SIZE - tile_y_offset - 1;
        }

        bg_color_id = TILE_PIXEL_COLORID(tile, tile_x_offset, tile_y_offset);
        palette = BG_PALETTE_READ(graphic->mem, TILE_ATTR_PALETTE(attr));

        bg_color = palette->c[bg_color_id];

        /* https://gbdev.io/pandocs/Tile_Maps.html#bg-to-obj-priority-in-cgb-mode */
        if (TILE_ATTR_PRIORITY(attr))
            bgwin_priority |= 1;
    }

    /* window */
    if (lcdc & LCDC_WINDOW_ENABLE) {
        /* TODO:
        we doesn't wait until WY and WX conditions are met
        https://gbdev.io/pandocs/Scrolling.html#window */

        uint8_t window_x = IO_PORT_READ(graphic->mem, IO_PORT_WX) - 7;
        uint8_t window_y = IO_PORT_READ(graphic->mem, IO_PORT_WY);

        /* notice that window_x and window_y are always positive */
        if (scanline >= window_y && col >= window_x) {
            gbc_tilemap_t *win_tilemap = gbc_graphic_get_tilemap(graphic, TILE_TYPE_WIN);
            gbc_tilemap_attr_t *win_tilemap_attr = gbc_graphic_get_tilemap_attr(graphic, TILE_TYPE_WIN);

            x = col - window_x;
            y = scanline - window_y;

            tile_x = x / TILE_SIZE;
            tile_y = y / TILE_SIZE;

            tile_x_offset = x % TILE_SIZE;
            tile_y_offset = y % TILE_SIZE;

            attr = win_tilemap_attr->data[tile_y][tile_x];
            tile = gbc_graphic_get_tile(graphic, TILE_TYPE_WIN, win_tilemap->data[tile_y][tile_x],
                    TILE_ATTR_VRAM_BANK(attr) ? 1 : 0);

            if (TILE_ATTR_XFLIP(attr)) {
                tile_x_offset = TILE_SIZE - tile_x_offset - 1;
            }

            if (TILE_ATTR_YFLIP(attr)) {
                tile_y_offset = TILE_SIZE - tile_y_offset - 1;
            }

            bg_color_id = TILE_PIXEL_COLORID(tile, tile_x_offset, tile_y_offset);
            palette = BG_PALETTE_READ(graphic->mem, TILE_ATTR_PALETTE(attr));

            /* window has higher priority than background */
            bg_color = palette->c[bg_color_id];

            if (TILE_ATTR_PRIORITY(attr))
                bgwin_priority |= 1;
        }
    }

    if (lcdc_bit0 && bg_color_id && bgwin_priority) {
        return bg_color;
    }

    uint8_t obj_found = 0;

    /* objs */
    if (lcdc & LCDC_OBJ_ENABLE) {
        for (int i = 0; i < objs_count; i++) {
            gbc_obj_t *obj = (gbc_obj_t*)OAM_ADDR(graphic->mem);
            obj += objs_idx[i];

            int16_t obj_y = OAM_Y_TO_SCREEN(obj->y);
            int16_t obj_x = OAM_X_TO_SCREEN(obj->x);

            if (col < obj_x || col >= obj_x + OBJ_WIDTH) {
                continue;
            }

            uint8_t tile_idx = obj->tile;
            uint8_t tile_x_offset = col - obj_x;
            uint8_t tile_y_offset = scanline - obj_y;

            if (lcdc & LCDC_OBJ_SIZE) {
                /* 8x16 */
                if (scanline >= obj_y + OBJ_HEIGHT) {
                    /* bottom tile */
                    tile_y_offset -= TILE_SIZE;
                    tile_idx = OBJ_ATTR_YFLIP(obj->attr) ? (tile_idx & 0xFE) : (tile_idx | 0x01);
                } else {
                    /* top tile */
                    tile_idx = OBJ_ATTR_YFLIP(obj->attr) ? (tile_idx | 0x01) : (tile_idx & 0xFE);
                }
            }

            uint8_t attr = obj->attr;
            gbc_tile_t *tile = gbc_graphic_get_tile(graphic, TILE_TYPE_OBJ, tile_idx,
                OBJ_ATTR_VRAM_BANK(attr) ? 1 : 0);

            if (OBJ_ATTR_XFLIP(attr)) {
                tile_x_offset = TILE_SIZE - tile_x_offset - 1;
            }

            if (OBJ_ATTR_YFLIP(attr)) {
                tile_y_offset = TILE_SIZE - tile_y_offset - 1;
            }

            assert(tile_x_offset >= 0 && tile_x_offset < TILE_SIZE);

            uint16_t color_id = TILE_PIXEL_COLORID(tile, tile_x_offset, tile_y_offset);

            if (color_id == 0) {
                /* color 0 means transparent */
                continue;
            }

            gbc_palette_t *palette = OBJ_PALETTE_READ(graphic->mem, OBJ_ATTR_PALETTE(attr));
            obj_color = palette->c[color_id];
            if (OBJ_ATTR_BG_PRIORITY(attr))
                bgwin_priority |= 1;
            /*
            the earlier(mem position in OAM) obj has higher priority
            and gameboy doest have alpha channel, we can break here
            */
            obj_found = 1;
            break;
        }
    }

    if (obj_found && (!bgwin_priority || !bg_color_id || !lcdc_bit0)) {
        return obj_color;
    }

    return bg_color;
}

static void
gbc_graphic_draw_line(gbc_graphic_t *graphic, uint16_t scanline)
{
    int16_t scanline_base = scanline * VISIBLE_HORIZONTAL_PIXELS;

    /* scan objs */
    uint8_t objs = 0;
    uint8_t lcdc = IO_PORT_READ(graphic->mem, IO_PORT_LCDC);
    uint8_t obj_height = lcdc & LCDC_OBJ_SIZE ? OBJ_HEIGHT_2 : OBJ_HEIGHT;

    gbc_obj_t *obj = (gbc_obj_t*)OAM_ADDR(graphic->mem);
    uint8_t objs_idx[MAX_OBJ_SCANLINE];

    int16_t signed_scanline = (int16_t)scanline;

    for (int i = 0; i < MAX_OBJS; i++, obj++) {
        int16_t y = OAM_Y_TO_SCREEN(obj->y);

        if (signed_scanline >= y && signed_scanline < y + obj_height) {
            objs_idx[objs++] = i;
            if (objs == MAX_OBJ_SCANLINE) {
                break;
            }
        }
    }

    for (int16_t i = 0; i < VISIBLE_HORIZONTAL_PIXELS; i++) {
        uint16_t color = gbc_graphic_render_pixel(graphic, scanline, i, objs_idx, objs);
        graphic->screen_write(graphic->screen_udata, scanline_base + i, color);
    }
}

void
gbc_graphic_cycle(gbc_graphic_t *graphic)
{
    if (graphic->dots--) {
        return;
    }

    uint8_t io_lcdc = IO_PORT_READ(graphic->mem, IO_PORT_LCDC);

    if (io_lcdc & LCDC_PPU_ENABLE) {
        uint8_t io_stat = IO_PORT_READ(graphic->mem, IO_PORT_STAT);
        uint8_t scanline = graphic->scanline;

        if (scanline <= VISIBLE_SCANLINES) {
            if (graphic->mode == PPU_MODE_3) {
                /* HORIZONTAL BLANK */
                graphic->dots = PPU_MODE_0_DOTS;
                graphic->mode = PPU_MODE_0;
                if (io_stat & STAT_MODE_0_INT) {
                    REQUEST_INTERRUPT(graphic->mem, INTERRUPT_LCD_STAT);
                }

            } else if (graphic->mode == PPU_MODE_2) {
                /* DRAWING */
                graphic->dots = PPU_MODE_3_DOTS;
                graphic->mode = PPU_MODE_3;
                gbc_graphic_draw_line(graphic, scanline);
            } else if (graphic->mode == PPU_MODE_0 || graphic->mode == PPU_MODE_1) {
                if (graphic->mode != PPU_MODE_1)
                    scanline++;
                /* OAM SCAN */
                /* The real gameboy scans obj here but we scan then in MODE3, see above */
                graphic->dots = PPU_MODE_2_DOTS;
                graphic->mode = PPU_MODE_2;
                if (io_stat & STAT_MODE_2_INT) {
                    REQUEST_INTERRUPT(graphic->mem, INTERRUPT_LCD_STAT);
                }
            }
        } else {
            /* V-BLANK */
            if (graphic->mode != PPU_MODE_1) {
                if (io_stat & STAT_MODE_1_INT) {
                    REQUEST_INTERRUPT(graphic->mem, INTERRUPT_LCD_STAT);
                }
                REQUEST_INTERRUPT(graphic->mem, INTERRUPT_VBLANK);
                graphic->mode = PPU_MODE_1;
            }

            graphic->dots = PPU_MODE_1_DOTS;
            scanline++;
        }

        if (scanline > TOTAL_SCANLINES)
            scanline = 0;

        io_stat &= ~PPU_MODE_MASK;
        io_stat |= graphic->mode & PPU_MODE_MASK;

        if (graphic->scanline != scanline) {
            graphic->scanline = scanline;
            IO_PORT_WRITE(graphic->mem, IO_PORT_LY, scanline);
            uint8_t lyc = IO_PORT_READ(graphic->mem, IO_PORT_LYC);
            io_stat &= ~STAT_LYC_LY;
            if (lyc == scanline) {
                io_stat |= STAT_LYC_LY;
                if (io_stat & STAT_LYC_INT) {
                    REQUEST_INTERRUPT(graphic->mem, INTERRUPT_LCD_STAT);
                }
            }
        }

        IO_PORT_WRITE(graphic->mem, IO_PORT_STAT, io_stat);
    } else {
        /* otherwise games like Metal Gear Solid will enter a infinite loop
          Hits to debug:
          After chose the difficulty, the game will at some point execute to
          0x402a then 0xe63, which has a instruction to enable interrupt,
          it will be interrupt to 0x48(LCD interrupt).
          After a few further jumps, it will jump to 0x42cb(ROM 0x65), which checks
          if LY == 0, if not, it will continue to execute the following instruction,
          which has a loop to check if LY == LYC, but the LCD is disabled, so it will
          never be true, and the game will stuck here.
          I thus suspect that the LY should be zero if the LCD is disabled. But I didn't
          find it in pandoc and gbdev. I googled "gameboy color ly lcdc disabled" and the
          first result is:
          /* https://www.reddit.com/r/Gameboy/comments/a1c8h0/what_happens_when_a_gameboy_screen_is_disabled/
          which saved my day.
          I am very tired when writing this, but with a sense of accomplishment.
        */
        IO_PORT_WRITE(graphic->mem, IO_PORT_LY, 0);
        graphic->dots = DOTS_PER_SCANLINE * TOTAL_SCANLINES;
        LOG_DEBUG("[GRAPHIC] PPU DISABLED\n");
    }
}

inline static void*
vram_addr_bank(void *udata, uint16_t addr, uint8_t bank)
{
    gbc_graphic_t *graphic = (gbc_graphic_t*)udata;
    uint16_t real_addr = (bank * VRAM_BANK_SIZE) + addr - VRAM_BEGIN;
    return graphic->vram + real_addr;
}

inline static void*
vram_addr(void *udata, uint16_t addr)
{
    gbc_graphic_t *graphic = (gbc_graphic_t*)udata;
    uint8_t bank = IO_PORT_READ(graphic->mem, IO_PORT_VBK) & 0x01;
    // LOG_DEBUG("[GRAPHIC] Reading from VRAM %x, bank: %d\n", addr, bank);
    return vram_addr_bank(udata, addr, bank);
}

static uint8_t
vram_read(void *udata, uint16_t addr)
{
    return *(uint8_t*)vram_addr(udata, addr);
}

static uint8_t
vram_write(void *udata, uint16_t addr, uint8_t data)
{
    gbc_graphic_t *graphic = (gbc_graphic_t*)udata;
    uint8_t bank = IO_PORT_READ(graphic->mem, IO_PORT_VBK) & 0x01;
    // LOG_DEBUG("[GRAPHIC] Writing to VRAM %x [%x], bank: %d\n", addr, data, bank);
    *(uint8_t*)vram_addr(udata, addr) = data;

    return data;
}

void
gbc_graphic_connect(gbc_graphic_t *graphic, gbc_memory_t *mem)
{
    graphic->mem = mem;

    memory_map_entry_t entry;
    entry.id = VRAM_ID;
    entry.addr_begin = VRAM_BEGIN;
    entry.addr_end = VRAM_END;
    entry.read = vram_read;
    entry.write = vram_write;
    entry.udata = graphic;

    register_memory_map(mem, &entry);
}