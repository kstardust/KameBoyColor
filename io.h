#ifndef _IO_H
#define _IO_H

#include "common.h"
#include "memory.h"

typedef struct gbc_io gbc_io_t;


struct gbc_io
{    
    gbc_memory_t *mem;
};

void gbc_io_connect(gbc_io_t *io, gbc_memory_t *mem);
void gbc_io_init(gbc_io_t *io);
void gbc_io_cycle(gbc_io_t *io);

#endif