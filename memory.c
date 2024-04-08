#include "memory.h"

uint8_t 
mem_write(uint16_t addr, uint8_t data)
{
    printf("Writing to memory at address %x [%x]\n", addr, data);
    return 0;
}

uint8_t 
mem_read(uint16_t addr)
{
    printf("Reading from memory at address %x\n", addr);
    return 0;
}

gbc_memory_t
gbc_mem_new()
{
    gbc_memory_t mem;
    mem.write = mem_write;
    mem.read = mem_read;
    return mem;
}
