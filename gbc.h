#ifndef _GBC_H
#define _GBC_H

#include "cpu.h"
#include "memory.h"

typedef struct gbc gbc_t;

struct gbc {
    gbc_cpu_t cpu;
    gbc_memory_t mem;
};

gbc_t gbc_new();

#endif