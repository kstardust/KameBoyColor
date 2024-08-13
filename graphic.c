#include "graphic.h"
#include "memory.h"
#include "cpu.h"

static void* vram_addr(void *udata, uint16_t addr);
static void* vram_addr_bank(void *udata, uint16_t addr, uint8_t bank);
static void* oam_addr(void *udata, uint16_t addr);

/* each tile is 8x8, top-left is 0,0 */
#define TILE_PIXEL_COLORID(td, x, y)  \
    ((td)->data[y * 2] & (1 << (7 - x)) ? 1 : 0) + \
    ((td)->data[y * 2 + 1] & (1 << (7 - x)) ? 2 : 0)


void 
gbc_graphic_init(gbc_graphic_t *graphic)
{    
    memset(graphic, 0, sizeof(gbc_graphic_t));    
}

gbc_tile_attr_t*
gbc_graphic_get_tile_attr(gbc_graphic_t *graphic, uint8_t type, uint8_t idx) 
{
    if (type == TILE_TYPE_OBJ || 
        (IO_PORT_READ(graphic->mem, IO_PORT_LCDC) & LCDC_BG_WINDOW_TILE_DATA)) {
        return (gbc_tile_attr_t*)(oam_addr(graphic, 0x8000) + idx * 16);
    }    
    return (gbc_tile_attr_t*)oam_addr(graphic, 0x9000 + (int8_t)idx * 16);
}

gbc_tile_t*
gbc_graphic_get_tile(gbc_graphic_t *graphic, uint8_t type, uint8_t idx)
{
    if (type == TILE_TYPE_OBJ || 
        (IO_PORT_READ(graphic->mem, IO_PORT_LCDC) & LCDC_BG_WINDOW_TILE_DATA)) {
        return (gbc_tile_t*)(vram_addr(graphic, 0x8000) + idx * 16);
    }    
    return (gbc_tile_t*)vram_addr(graphic, 0x9000 + (int8_t)idx * 16);
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

inline static uint8_t
gbc_graphic_render_pixel(gbc_graphic_t *graphic, uint16_t scanline, uint16_t col)
{
    uint8_t lcdc = IO_PORT_READ(graphic->mem, IO_PORT_LCDC);

    /* background */
    /* TODO: 
    "The scroll registers are re-read on each tile fetch, except for the low 3 bits of SCX" Does it matter?  
    https://gbdev.io/pandocs/Scrolling.html#mid-frame-behavior 
    */
    uint16_t scroll_x = IO_PORT_READ(graphic->mem, IO_PORT_SCX);
    uint16_t scroll_y = IO_PORT_READ(graphic->mem, IO_PORT_SCY);
    gbc_tilemap_t *bg_tilemap = gbc_graphic_get_tilemap(graphic, TILE_TYPE_BG);
    //gbc_tilemap_attr_t *bg_tilemap_attr = gbc_graphic_get_tilemap_attr(graphic, TILE_TYPE_BG);

    uint16_t x = (scroll_x + col) % 256;
    uint16_t y = (scroll_y + scanline) % 256;

    uint16_t tile_x = x / 8;
    uint16_t tile_y = y / 8;

    uint16_t tile_x_offset = x % 8;
    uint16_t tile_y_offset = y % 8;

    gbc_tile_t *tile = gbc_graphic_get_tile(graphic, TILE_TYPE_BG, bg_tilemap->data[tile_y][tile_x]);
    uint8_t color_id = TILE_PIXEL_COLORID(tile, tile_x_offset, tile_y_offset);

    return color_id;
    /* window */

    /* objs */
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

    return (gbc_tilemap_attr_t*)vram_addr_bank(graphic, addr, 1);
}

static void 
gbc_graphic_draw_line(gbc_graphic_t *graphic, uint16_t scanline)
{   
    int16_t scanline_base = scanline * VISIBLE_HORIZONTAL_PIXELS;    
    for (uint16_t i = 0; i < VISIBLE_HORIZONTAL_PIXELS; i++) {
        uint8_t color = gbc_graphic_render_pixel(graphic, scanline, i);
        graphic->screen_write(graphic->screen_udata, scanline_base + i, color);
    }
}

void 
gbc_graphic_cycle(gbc_graphic_t *graphic, uint64_t delta)
{
    /* TODO: it's still too slow */
    graphic->t_delta += delta;
    
    if (graphic->t_delta < GRAPHIC_UPDATE_TIME) {        
        return;
    }

    uint8_t io_lcdc = IO_PORT_READ(graphic->mem, IO_PORT_LCDC);
    
    if (io_lcdc & LCDC_PPU_ENABLE) {
        graphic->t_delta = 0;
        uint8_t io_stat = IO_PORT_READ(graphic->mem, IO_PORT_STAT);
        uint8_t scanline_cycle = graphic->scanline_cycles;
        uint8_t scanline = graphic->scanline;

        if (scanline <= VISIBLE_SCANLINES) {
            if (scanline_cycle > CYCLE_MODEL_0_START) {
                /* HORIZONTAL BLANK */
                if (graphic->mode != PPU_MODE_0) {
                    graphic->mode = PPU_MODE_0;
                    if (io_stat & STAT_MODE_0_INT) {
                        REQUEST_INTERRUPT(graphic->mem, INTERRUPT_LCD_STAT);
                    }                    
                }                
            } else if (scanline_cycle > CYCLE_MODEL_3_START) {
                /* DRAWING  */                
                if (graphic->mode != PPU_MODE_3) {
                    graphic->mode = PPU_MODE_3;
                    gbc_graphic_draw_line(graphic, scanline);                    
                }                
            } else if (scanline_cycle > CYCLE_MODEL_2_START) {
                /* OAM SCAN */
                if (graphic->mode != PPU_MODE_2) {
                    graphic->mode = PPU_MODE_2;
                    if (io_stat & STAT_MODE_2_INT) {
                        REQUEST_INTERRUPT(graphic->mem, INTERRUPT_LCD_STAT);
                    }
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
        }
    
        scanline_cycle++;
        
        if (scanline_cycle == CYCLES_PER_SCANLINE) {
            scanline_cycle = 0;
            scanline++;
            if (scanline > TOTAL_SCANLINES) {                
                graphic->screen_update(graphic->screen_udata);
                scanline = 0;
            }
        }
        
        graphic->scanline_cycles = scanline_cycle;
        graphic->scanline = scanline;    

        uint8_t lyc = IO_PORT_READ(graphic->mem, IO_PORT_LYC);

        IO_PORT_WRITE(graphic->mem, IO_PORT_LY, scanline);    

        io_stat &= ~PPU_MODE_MASK;
        io_stat |= graphic->mode & PPU_MODE_MASK;

        io_stat &= ~STAT_LYC_LY;
        if (lyc == scanline) {
            io_stat |= STAT_LYC_LY;
            if (io_stat & STAT_LYC_INT) {
                REQUEST_INTERRUPT(graphic->mem, INTERRUPT_LCD_STAT);
            }
        }

        IO_PORT_WRITE(graphic->mem, IO_PORT_STAT, io_stat);        
    } else  {        
        if (graphic->t_delta < GRAPHIC_UPDATE_TIME * CYCLES_PER_SCANLINE * TOTAL_SCANLINES) {
            return;
        }        
        graphic->screen_update(graphic->screen_udata);
        graphic->t_delta = 0;
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

static void*
oam_addr(void *udata, uint16_t addr)
{
    // LOG_DEBUG("[GRAPHIC] Reading from OAM %x\n", addr);
    gbc_graphic_t *graphic = (gbc_graphic_t*)udata;

    uint16_t real_addr = addr - OAM_BEGIN;
    return graphic->oam + real_addr;
}

static uint8_t
oam_read(void *udata, uint16_t addr)
{
    return *(uint8_t*)oam_addr(udata, addr);
}

static uint8_t
oam_write(void *udata, uint16_t addr, uint8_t data)
{
    // LOG_DEBUG("[GRAPHIC] Writing to OAM %x [%x]\n", addr, data);
    gbc_graphic_t *graphic = (gbc_graphic_t*)udata;
    
    uint16_t real_addr = addr - OAM_BEGIN;
    graphic->oam[real_addr] = data;
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

    entry.id = OAM_ID;
    entry.addr_begin = OAM_BEGIN;
    entry.addr_end = OAM_END;
    entry.read = oam_read;
    entry.write = oam_write;
    entry.udata = graphic;

    register_memory_map(mem, &entry);
}