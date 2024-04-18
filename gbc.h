#ifndef _GBC_H
#define _GBC_H

#include "cpu.h"
#include "memory.h"
#include "mbc.h"

typedef struct gbc gbc_t;

struct gbc {
    gbc_cpu_t cpu;
    gbc_memory_t mem;
    gbc_mbc_t mbc;
};

void gbc_init(gbc_t *gbc);

#endif