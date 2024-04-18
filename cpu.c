#include <string.h>
#include "cpu.h"
#include "instruction_set.h"

gbc_cpu_t 
gbc_cpu_new()
{
    gbc_cpu_t cpu;
    memset(&cpu, 0, sizeof(cpu.regs));
    return cpu;
}

void
gbc_cpu_connect(gbc_cpu_t *cpu, gbc_memory_t *mem)
{
    cpu->mem_data = mem;
    cpu->mem_read = mem->read;
    cpu->mem_write = mem->write;    
}

cpu_cycle(gbc_cpu_t *cpu)
{
}