#ifndef _MEMORY_H
#define _MEMORY_H

#include "common.h"

typedef uint8_t (*memory_read)(uint16_t addr);
typedef uint8_t (*memory_write)(uint16_t addr, uint8_t data);

typedef struct gbc_memory gbc_memory_t;
typedef struct memory_bank memory_bank_t;

struct gbc_memory
{
    memory_read read;
    memory_write write;
    uint8_t *data;
};

struct memory_bank
{    
    uint8_t id;
    uint16_t size;
    memory_read read;
    memory_write write;
    uint8_t *data;
};

gbc_memory_t gbc_mem_new();

#endif 
