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
