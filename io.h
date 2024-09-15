#ifndef _IO_H
#define _IO_H

#include "common.h"
#include "memory.h"

#define GBC_KEY_BUTTON  0x20
#define GBC_KEY_DPAD    0x10

#define GBC_KEY_A       0x01
#define GBC_KEY_B       0x02
#define GBC_KEY_SELECT  0x04
#define GBC_KEY_START   0x08

#define GBC_KEY_RIGHT   0x10
#define GBC_KEY_LEFT    0x20
#define GBC_KEY_UP      0x40
#define GBC_KEY_DOWN    0x80

typedef struct gbc_io gbc_io_t;

struct gbc_io
{    
    gbc_memory_t *mem;
    uint8_t (*poll_keypad)();
};

void gbc_io_connect(gbc_io_t *io, gbc_memory_t *mem);
void gbc_io_init(gbc_io_t *io);
void gbc_io_cycle(gbc_io_t *io);

#endif