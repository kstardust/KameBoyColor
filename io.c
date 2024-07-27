#include "io.h"

void
gbc_io_connect(gbc_io_t *io, gbc_memory_t *mem)
{
    io->mem = mem;
}

void
gbc_io_init(gbc_io_t *io)
{
    memset(io, 0, sizeof(gbc_io_t));
}

void 
gbc_io_cycle(gbc_io_t *io)
{
    uint8_t sc = IO_PORT_READ(io->mem, IO_PORT_SC);
    uint8_t sb = IO_PORT_READ(io->mem, IO_PORT_SB);

    /* TODO: THIS IS FOR DEBUGGING PURPOSES */    
    if (sc == 0x81) {
        IO_PORT_WRITE(io->mem, IO_PORT_SC, 0x01);
        fprintf(stderr, "%c", sb);
    }
}