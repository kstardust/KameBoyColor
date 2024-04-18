#include "memory.h"

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
    printf("Writing to memory at address %x [%x]\n", addr, data);
    gbc_memory_t *mem = (gbc_memory_t*)udata;
    memory_map_entry_t *entry = select_entry(mem, addr);

    if (entry == NULL) {
        LOG_ERROR("No memory map entry found for address %x\n", addr);
        abort();
    }

    return entry->write(entry->udata, addr, data);
}

uint8_t 
mem_read(void *udata, uint16_t addr)
{
    printf("Reading from memory at address %x\n", addr);
    gbc_memory_t *mem = (gbc_memory_t*)udata;
    memory_map_entry_t *entry = select_entry(mem, addr);

    if (entry == NULL) {
        LOG_ERROR("No memory map entry found for address %x\n", addr);
        abort();
    }    
    
    return entry->read(entry->udata, addr);
}

void
register_memory_map(gbc_memory_t *mem, memory_map_entry_t *entry)
{    
    LOG_INFO("Registering memory map entry with id %d [0x%x] - [0x%x]\n", entry->id, entry->addr_begin, entry->addr_end);

    if (entry->id >= MEMORY_MAP_ENTRIES) {
        LOG_ERROR("Memory map entry id %d is out of bounds\n", entry->id);
        abort();
    }

    if (entry->addr_begin > entry->addr_end) {
        LOG_ERROR("Memory map entry id %d has invalid address range [%x] - [%x]\n", entry->id, entry->addr_begin, entry->addr_end);
        abort();
    }

    if (mem->map[entry->id-1].id != 0) {
        LOG_ERROR("Memory map entry id %d is already registered\n", entry->id);
        abort();
    }

    for (int i = 0; i < MEMORY_MAP_ENTRIES; i++) {
        memory_map_entry_t *e = &mem->map[i];        
        if (e->id && e->addr_begin <= entry->addr_end && e->addr_end >= entry->addr_begin) {
            LOG_ERROR("Memory map entry id %d overlaps with existing entry id %d\n", entry->id, e->id);
            abort();
        }
    }

    mem->map[entry->id-1] = *entry;    
}

gbc_memory_t
gbc_mem_new()
{
    gbc_memory_t mem;
    memset(&mem, 0, sizeof(mem));

    mem.write = mem_write;
    mem.read = mem_read;
    
    return mem;
}