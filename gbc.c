#include "gbc.h"

gbc_t
gbc_new()
{
    gbc_t gbc;
    gbc.cpu = gbc_cpu_new();
    gbc.mem = gbc_mem_new();

    gbc.cpu.mem_read = gbc.mem.read;
    gbc.cpu.mem_write = gbc.mem.write;
    
    return gbc;
}