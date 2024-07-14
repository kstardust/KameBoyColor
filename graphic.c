#include "graphic.h"

static void* vram_addr(void *udata, uint16_t addr);
static void* oam_addr(void *udata, uint16_t addr);

/* each tile is 8x8, top-left is 0,0 */
#define PIXEL_TILE_INDEX(td, x, y)  \
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

static void 
gbc_graphic_draw_line(gbc_graphic_t *graphic, uint16_t scanline)
{       
    graphic->screen_write(graphic->screen_udata, 100*scanline, 111);    
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
                graphic->mode = PPU_MODE_0;
            } else if (scanline_cycle > CYCLE_MODEL_3_START) {
                /* DRAWING  */                
                if (graphic->mode != PPU_MODE_3) {
                    gbc_graphic_draw_line(graphic, scanline);
                }

                graphic->mode = PPU_MODE_3;                                
            } else if (scanline_cycle > CYCLE_MODEL_2_START) {
                /* OAM SCAN */
                graphic->mode = PPU_MODE_2;
            }
        } else {
            /* V-BLANK */        
            graphic->mode = PPU_MODE_1;
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

        io_stat &= ~0x04;
        if (lyc == scanline) {
            /* TODO stat interrupt */        
            io_stat |= 0x04;
        }

        IO_PORT_WRITE(graphic->mem, IO_PORT_STAT, io_stat);    
    } else  {        
        if (graphic->t_delta < GRAPHIC_UPDATE_TIME * CYCLES_PER_SCANLINE * TOTAL_SCANLINES) {
            return;
        }        
        graphic->screen_update(graphic->screen_udata);
        graphic->t_delta = 0;
    }
}

static void*
vram_addr(void *udata, uint16_t addr)
{
    gbc_graphic_t *graphic = (gbc_graphic_t*)udata;
    uint8_t bank = IO_PORT_READ(graphic->mem, IO_PORT_VBK) & 0x01;
    LOG_DEBUG("[GRAPHIC] Reading from VRAM %x, bank: %d\n", addr, bank);

    uint16_t real_addr = (bank * VRAM_BANK_SIZE) + addr - VRAM_BEGIN;

    return graphic->vram + real_addr;
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
    LOG_DEBUG("[GRAPHIC] Writing to VRAM %x [%x], bank: %d\n", addr, data, bank);

    uint16_t real_addr = (bank * VRAM_BANK_SIZE) + addr - VRAM_BEGIN;
    graphic->vram[real_addr] = data;

    return data;
}

static void*
oam_addr(void *udata, uint16_t addr)
{
    LOG_DEBUG("[GRAPHIC] Reading from OAM %x\n", addr);
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
    LOG_DEBUG("[GRAPHIC] Writing to OAM %x [%x]\n", addr, data);
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