#include "gbc.h"

void
gbc_init(gbc_t *gbc)
{
    gbc->cpu = gbc_cpu_new();
    gbc->mbc = gbc_mbc_new();
    gbc->mem = gbc_mem_new();

    gbc_cpu_connect(&gbc->cpu, &gbc->mem);
    gbc_mbc_connect(&gbc->mbc, &gbc->mem);
}
