#ifndef _INSTRUCTION_SET_H
#define _INSTRUCTION_SET_H

#include "gbc.h"

typedef struct instruction instruction_t;
typedef void (*instruction_func)(gbc_cpu_t *cpu, instruction_t *ins);

#define INSTRUCTIONS_SET_SIZE 512

#define PREFIX_CB 0xcb

#define INSTRUCTION_ADD(opcode, size, func, op1, op2, c1, c2, name) {(opcode), (size), (c1), (c2), (c1), (func), ((void*)(op1)), ((void*)(op2)), 0, (name)}

struct instruction
{   
    uint8_t opcode;
    uint8_t size;    
    uint8_t cycles;               /* default cost */
    uint8_t cycles2;              /* alternative cost */
    uint8_t r_cycles;             /* real cost */
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
instruction_t* decode(uint8_t *data);
void  test_instructions();
#endif
