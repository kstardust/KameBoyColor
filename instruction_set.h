#ifndef _INSTRUCTION_SET_H
#define _INSTRUCTION_SET_H

#include "gbc.h"

typedef struct instruction instruction_t;
typedef void (*instruction_func)(gbc_cpu_t *cpu, instruction_t *ins);

#define INSTRUCTIONS_SET_SIZE 512

#define PREFIX_CB 0xcb

#define INSTRUCTION_ADD(opcode, size, func, op1, op2, name) {(opcode), (size), (func), ((void*)(op1)), ((void*)(op2)), 0, (name)}

struct instruction
{   
    uint8_t opcode;    
    uint8_t size;
    instruction_func func;
    void *op1;
    void *op2;
    /* https://gbdev.io/gb-opcodes/optables/ */
    union {
        uint16_t i16;    /* little-endian 16-bit immediate */        
        uint8_t i8;      /* 8-bit immediate */
    } opcode_ext;    
    const char *name;
};

void init_instruction_set();
instruction_t decode(uint8_t *data);
void  test_instructions();
#endif
