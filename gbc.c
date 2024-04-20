#include "gbc.h"

void
gbc_init(gbc_t *gbc)
{
    gbc_cpu_init(&gbc->cpu);
    gbc_mbc_init(&gbc->mbc);    
    gbc_mem_init(&gbc->mem);

    gbc_cpu_connect(&gbc->cpu, &gbc->mem);
    gbc_mbc_connect(&gbc->mbc, &gbc->mem);
}
