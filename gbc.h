#ifndef _GBC_H
#define _GBC_H

#include "cpu.h"
#include "memory.h"
#include "io.h"
#include "mbc.h"
#include "graphic.h"
#include "timer.h"
#include "audio.h"

typedef struct gbc gbc_t;

struct gbc {
    gbc_cpu_t cpu;
    gbc_memory_t mem;
    gbc_mbc_t mbc;
    gbc_io_t io;
    gbc_graphic_t graphic;
    gbc_timer_t timer;
    gbc_audio_t audio;

    uint32_t debug_steps;
    volatile uint8_t running:1;
    volatile uint8_t paused:1;
};

int gbc_init(gbc_t *gbc, const char *game_rom, const char *boot_rom);
void gbc_run(gbc_t *gbc);

#endif