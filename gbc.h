#ifndef _GBC_H
#define _GBC_H

#include "cpu.h"
#include "memory.h"
#include "io.h"
#include "mbc.h"
#include "graphic.h"

typedef struct gbc gbc_t;

struct gbc {
    gbc_cpu_t cpu;
    gbc_memory_t mem;
    gbc_mbc_t mbc;
    gbc_io_t io;
    gbc_graphic_t graphic;
};

int gbc_init(gbc_t *gbc);
void gbc_run(gbc_t *gbc);

#endif