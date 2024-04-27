#include "graphic.h"

void 
gbc_graphic_init(gbc_graphic_t *graphic)
{    
    memset(graphic, 0, sizeof(gbc_graphic_t));    
}

static uint8_t
vram_read(void *udata, uint16_t addr)
{
    LOG_DEBUG("[GRAPHIC] Reading from VRAM %x\n", addr);
    gbc_graphic_t *graphic = (gbc_graphic_t*)udata;
    return 0;
}

static uint8_t
vram_write(void *udata, uint16_t addr, uint8_t data)
{
    LOG_DEBUG("[GRAPHIC] Writing to VRAM %x [%x]\n", addr, data);
    gbc_graphic_t *graphic = (gbc_graphic_t*)udata;
    return 0;
}

static uint8_t
oam_read(void *udata, uint16_t addr)
{
    LOG_DEBUG("[GRAPHIC] Reading from OAM %x\n", addr);
    gbc_graphic_t *graphic = (gbc_graphic_t*)udata;
    return 0;
}

static uint8_t
oam_write(void *udata, uint16_t addr, uint8_t data)
{
    LOG_DEBUG("[GRAPHIC] Writing to OAM %x [%x]\n", addr, data);
    gbc_graphic_t *graphic = (gbc_graphic_t*)udata;
    return 0;
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