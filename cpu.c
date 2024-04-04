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
cpu_cycle(gbc_cpu_t *cpu)
{
}
