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
poll_keypad(void *udata)
{
    gbc_io_t *io = (gbc_io_t*)udata;
    uint8_t p1 = IO_PORT_READ(io->mem, IO_PORT_P1);
    uint8_t key = io->poll_keypad();

    /* https://gbdev.io/pandocs/Joypad_Input.html#ff00--p1joyp-joypad */
    /* If DPAD bit IS ZERO, then directional keys can be read */
    if (!(p1 & GBC_KEY_DPAD))
        key >>= 4;
    key = ~key & 0xF;
    p1 = (p1 & 0xF0) | key;
    IO_PORT_WRITE(io->mem, IO_PORT_P1, p1);
}

void 
gbc_io_cycle(gbc_io_t *io)
{
    uint8_t sc = IO_PORT_READ(io->mem, IO_PORT_SC);
    uint8_t sb = IO_PORT_READ(io->mem, IO_PORT_SB);

    poll_keypad(io);

    /* TODO: THIS IS FOR DEBUGGING PURPOSES */    
    if (sc == 0x81) {
        IO_PORT_WRITE(io->mem, IO_PORT_SC, 0x01);
        fprintf(stderr, "%c", sb);
    }
}