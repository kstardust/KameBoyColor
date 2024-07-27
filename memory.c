#include "memory.h"
#include "graphic.h"

memory_map_entry_t*
select_entry(gbc_memory_t *mem, uint16_t addr)
{ 
    for (int i = 0; i < MEMORY_MAP_ENTRIES; i++) {
        memory_map_entry_t *entry = &(mem->map[i]);        
        if (addr >= entry->addr_begin && addr <= entry->addr_end) {       
            if (entry->id == 0) {
                return NULL;                
            }
            return entry;
        }
    }
    return NULL;
}

uint8_t
mem_write(void *udata, uint16_t addr, uint8_t data)
{
    LOG_DEBUG("[MEM] Writing to memory at address %x [%x]\n", addr, data);
    gbc_memory_t *mem = (gbc_memory_t*)udata;
    memory_map_entry_t *entry = select_entry(mem, addr);

    if (entry == NULL) {
        LOG_ERROR("[MEM] No memory map entry found for address %x\n", addr);
        abort();
    }

    return entry->write(entry->udata, addr, data);
}

uint8_t 
mem_read(void *udata, uint16_t addr)
{
    LOG_DEBUG("[MEM] Reading from memory at address %x\n", addr);

    gbc_memory_t *mem = (gbc_memory_t*)udata;
    memory_map_entry_t *entry = select_entry(mem, addr);

    if (entry == NULL) {
        LOG_ERROR("[MEM] No memory map entry found for address %x\n", addr);
        abort();
    }    

    return entry->read(entry->udata, addr);
}

void
register_memory_map(gbc_memory_t *mem, memory_map_entry_t *entry)
{    
    LOG_DEBUG("[MEM] Registering memory map entry with id %d [0x%x] - [0x%x]\n", entry->id, entry->addr_begin, entry->addr_end);

    if (entry->id > MEMORY_MAP_ENTRIES) {
        LOG_ERROR("[MEM] Memory map entry id %d is out of bounds\n", entry->id);
        abort();
    }

    if (entry->addr_begin > entry->addr_end) {
        LOG_ERROR("[MEM] Memory map entry id %d has invalid address range [%x] - [%x]\n", entry->id, entry->addr_begin, entry->addr_end);
        abort();
    }

    if (mem->map[entry->id-1].id != 0) {
        LOG_ERROR("[MEM] Memory map entry id %d is already registered\n", entry->id);
        abort();
    }

    for (int i = 0; i < MEMORY_MAP_ENTRIES; i++) {
        memory_map_entry_t *e = &mem->map[i];        
        if (e->id && e->addr_begin <= entry->addr_end && e->addr_end >= entry->addr_begin) {
            LOG_ERROR("[MEM] Memory map entry id %d overlaps with existing entry id %d\n", entry->id, e->id);
            abort();
        }
    }

    mem->map[entry->id-1] = *entry;    
}

uint8_t
mem_raw_write(void *udata, uint16_t addr, uint8_t data)
{
    gbc_memory_t *mem = (gbc_memory_t*)udata;

    if (IN_RANGE(addr, HRAM_BEGIN, HRAM_END)) {
        mem->hraw[addr - HRAM_BEGIN] = data;
        return data;

    } else if (IN_RANGE(addr, WRAM_BANK_0_BEGIN, WRAM_BANK_0_END)) {
        mem->wram[addr - WRAM_BANK_0_BEGIN] = data;
        return data;

    }
    
    LOG_ERROR("[MEM] Invalid write, %x is not a valid WRAM addr \n", addr);
    abort();
}

uint8_t
mem_raw_read(void *udata, uint16_t addr)
{
    gbc_memory_t *mem = (gbc_memory_t*)udata;

    if (IN_RANGE(addr, HRAM_BEGIN, HRAM_END)) {
        return mem->hraw[addr - HRAM_BEGIN];

    } else if (IN_RANGE(addr, WRAM_BANK_0_BEGIN, WRAM_BANK_0_END)) {
        return mem->wram[addr - WRAM_BANK_0_BEGIN];

    }

    LOG_ERROR("[MEM] Invalid read, %x is not a valid WRAM addr \n", addr);
    abort();    
}

uint8_t
mem_echo_write(void *udata, uint16_t addr, uint8_t data)
{
    gbc_memory_t *mem = (gbc_memory_t*)udata;
    addr = addr - WRAM_ECHO_BEGIN + WRAM_BANK_0_BEGIN;
    return mem_raw_write(mem, addr, data);
}

uint8_t
mem_echo_read(void *udata, uint16_t addr)
{
    gbc_memory_t *mem = (gbc_memory_t*)udata;
    addr = addr - WRAM_ECHO_BEGIN + WRAM_BANK_0_BEGIN;
    return mem_raw_read(mem, addr);
}

uint8_t
io_port_read(void *udata, uint16_t addr)
{
    LOG_DEBUG("[MEM] Reading from IO port at address %x\n", addr);    
    return IO_PORT_READ((gbc_memory_t*)udata, IO_ADDR_PORT(addr));
}

uint8_t
io_port_write(void *udata, uint16_t addr, uint8_t data)
{
    LOG_DEBUG("[MEM] Writing to IO port at address %x [%x]\n", addr, data);
    IO_PORT_WRITE((gbc_memory_t*)udata, IO_ADDR_PORT(addr), data);
    return data;
}

uint8_t
bank_n_write(void *udata, uint16_t addr, uint8_t data)
{    
    gbc_memory_t *mem = (gbc_memory_t*)udata;
    uint8_t bank = IO_PORT_READ(mem, IO_PORT_SVBK) & 0x7;

    LOG_DEBUG("[MEM] Writing to switchable RAM bank [%x] at address %x [%x]\n", bank, addr, data);    

    uint16_t offset = WRAM_BANK_N_BEGIN + (bank * WRAM_BANK_SIZE);
    mem->wram[addr - offset] = data; 
    return data;
}

uint8_t
bank_n_read(void *udata, uint16_t addr)
{
    gbc_memory_t *mem = (gbc_memory_t*)udata;
    uint8_t bank = IO_PORT_READ(mem, IO_PORT_SVBK) & 0x7;

    LOG_DEBUG("[MEM] Reading from switchable RAM bank [%x] at address %x\n", bank, addr);
    
    uint16_t offset = WRAM_BANK_N_BEGIN + (bank * WRAM_BANK_SIZE);
    return mem->wram[addr - offset];
}

uint8_t
not_usable_write(void *udata, uint16_t addr, uint8_t data)
{    
    LOG_ERROR("[MEM] Writing to Not-Usable memory at address %x [%x]\n", addr, data);
    /* I have not found any information on what happens when writing to this memory */
    return 0;
}

uint8_t
not_usable_read(void *udata, uint16_t addr)
{
    /* It is actually readable, this implementation emulates CGB revision E */
    LOG_DEBUG("[MEM] Reading from Not-Usable memory at address %x\n", addr);    
    
    uint8_t lcdsr = IO_PORT_READ((gbc_memory_t*)udata, IO_PORT_STAT);

    uint8_t mode = lcdsr & PPU_MODE_MASK;
    if (mode == PPU_MODE_2 || mode == PPU_MODE_3) {
        return 0xff;
    }

    addr = addr & 0xf0;
    addr |= addr >> 4;
    return addr;
}

void
gbc_mem_init(gbc_memory_t *mem)
{
    memset(mem, 0, sizeof(gbc_memory_t));

    mem->write = mem_write;
    mem->read = mem_read;

    /* WRAM */
    memory_map_entry_t entry;
    entry.id = WRAM_BANK_0_ID;
    entry.addr_begin = WRAM_BANK_0_BEGIN;
    entry.addr_end = WRAM_BANK_0_END;
    entry.read = mem_raw_read;
    entry.write = mem_raw_write;
    entry.udata = mem;

    register_memory_map(mem, &entry);

    /* GBC switchable RAM bank */
    entry.id = WRAM_BANK_N_ID;
    entry.addr_begin = WRAM_BANK_N_BEGIN;
    entry.addr_end = WRAM_BANK_N_END;
    entry.read = bank_n_read;    
    entry.write = bank_n_write;
    entry.udata = mem;

    register_memory_map(mem, &entry);

    /* High RAM */
    entry.id = HRAM_ID;
    entry.addr_begin = HRAM_BEGIN;
    entry.addr_end = HRAM_END;
    entry.read = mem_raw_read;
    entry.write = mem_raw_write;
    entry.udata = mem;

    register_memory_map(mem, &entry);    

    /* IO Port */
    entry.id = IO_PORT_ID;
    entry.addr_begin = IO_PORT_BEGIN;
    entry.addr_end = IO_PORT_END;
    entry.read = io_port_read;
    entry.write = io_port_write;
    entry.udata = mem;

    register_memory_map(mem, &entry);

    /* 0xFEA0 - 0xFEFF, CGB revision E */
    entry.id = IO_NOT_USABLE_ID;
    entry.addr_begin = IO_NOT_USABLE_BEGIN;
    entry.addr_end = IO_NOT_USABLE_END;
    entry.read = not_usable_read;
    entry.write = not_usable_write;
    entry.udata = mem;

    register_memory_map(mem, &entry);

    /* Echo RAM */
    entry.id = WRAM_ECHO_ID;
    entry.addr_begin = WRAM_ECHO_BEGIN;
    entry.addr_end = WRAM_ECHO_END;
    
    entry.read = mem_echo_read;
    entry.write = mem_echo_write;
    entry.udata = mem;

    register_memory_map(mem, &entry);    
}