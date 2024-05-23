#ifndef _CPU_H
#define _CPU_H

#include <stdint.h>
#include "memory.h"
    
typedef struct cpu_register cpu_register_t;
typedef struct gbc_cpu gbc_cpu_t;

#define CLOCK_RATE 8388608                          /* 8.38 MHz */
#define CLOCK_CYCLE (1000000000 / CLOCK_RATE)     /* nanoseconds */

#define OFFSET_OF(type, field) \
    ((size_t) &((type *)0)->field)

#define OFFSET_OF_2(type, field, type2, field2) \
    (((size_t) &((type *)0)->field) + ((size_t) &((type2 *)0)->field2))

#define OFFSET_OF_3(type, field, type2, field2, type3, field3) \
    (((size_t) &((type *)0)->field) + ((size_t) &((type2 *)0)->field2) + ((size_t) &((type3 *)0)->field3))


#define REG_TYPE(high, low) union _reg_pair_##high##low
#define REG_FIELD_NAME(high, low) R_##high##low
#define REG_16_NAME(high, low) high##low
#define REG_PAIR_TYPE(high, low) struct _reg_##high##_##low
#define REG_PAIR_NAME(high, low) pair
#define REG_8_PAIR_FIELD_NAMe(high, low) high##low
#define REG_8_NAME(name) name

#define REGISTER_PAIR(high, low)                 \
    REG_TYPE(high, low)                          \
    {                                            \
        uint16_t REG_16_NAME(high, low);         \
        REG_PAIR_TYPE(high, low)                 \
        {                                        \
            uint8_t REG_8_NAME(low);             \
            uint8_t REG_8_NAME(high);            \
        } REG_PAIR_NAME(high, low);              \
    } REG_FIELD_NAME(high, low)


struct cpu_register
{
    REGISTER_PAIR(A, F);
    REGISTER_PAIR(B, C);
    REGISTER_PAIR(D, E);
    REGISTER_PAIR(H, L);
    uint16_t SP;
    uint16_t PC;        
    
    /* r16 */    
    #define _REG_16_OFFSET(h, l) OFFSET_OF_2(cpu_register_t, REG_FIELD_NAME(h, l), REG_TYPE(h, l), REG_16_NAME(h, l))
    #define REG_AF _REG_16_OFFSET(A, F)
    #define REG_BC _REG_16_OFFSET(B, C)
    #define REG_DE _REG_16_OFFSET(D, E)    
    #define REG_HL _REG_16_OFFSET(H, L)

    #define REG_SP OFFSET_OF(cpu_register_t, SP)
    #define REG_PC OFFSET_OF(cpu_register_t, PC)
    
    /* r8 */
    #define _REG_8_OFFSET(h, l, t) OFFSET_OF_3(cpu_register_t, REG_FIELD_NAME(h, l), REG_TYPE(h, l), REG_PAIR_NAME(h, l), REG_PAIR_TYPE(h, l), REG_8_NAME(t))
    #define REG_A _REG_8_OFFSET(A, F, A)
    #define REG_F _REG_8_OFFSET(A, F, F)
    #define REG_B _REG_8_OFFSET(B, C, B)
    #define REG_C _REG_8_OFFSET(B, C, C)
    #define REG_D _REG_8_OFFSET(D, E, D)
    #define REG_E _REG_8_OFFSET(D, E, E)
    #define REG_H _REG_8_OFFSET(H, L, H)
    #define REG_L _REG_8_OFFSET(H, L, L)    
};

struct gbc_cpu
{    
    cpu_register_t regs;    

    memory_read mem_read;  /* memory op */
    memory_write mem_write;
    void *mem_data;

    uint64_t cycles;
    uint16_t ins_cycles;   /* current instruction cost */    
    uint8_t ier;          /* interrupt enable */
};

#define swap_i16(value) (uint16_t)((value >> 8) | (value << 8));

//#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
/* https://www.c-faq.com/cpp/ifendian.html */
#define IS_LITTLE_ENDIAN 'ABCD' == 0x41424344
//#else
//#define IS_LITTLE_ENDIAN (((union { unsigned x; unsigned char c; }){1}).c)
//#endif

#if (IS_LITTLE_ENDIAN)
#define READ_I16(reg) reg                  /* immediate 16-bit */
#define READ_16(reg) reg
#define WRITE_16(reg, value) (reg) = (value)
#else
#define READ_I16(reg)
#define READ_16(reg) swap_i16(reg)
#define WRITE_16(reg, value) (reg) = swap_i16(value)
#endif

#define READ_I8(reg) (reg)                  /* immediate 8-bit */
#define READ_8(reg) (reg)
#define WRITE_8(reg, value) (reg) = (value)

#define READ_1(reg) (reg & 0x1)
#define WRITE_1(reg, value) (reg) = (value & 0x1)

#define READ_R16(reg, field) READ_16((*(uint16_t*) ((uint8_t*)(reg)+(field))))
#define WRITE_R16(reg, field, value) WRITE_16((*(uint16_t*) ((uint8_t*)(reg)+(field))), (value))

#define READ_R8(reg, field) READ_8(*(uint8_t*) ((uint8_t*)(reg)+(field)))
#define WRITE_R8(reg, field, value) WRITE_8((*(uint8_t*) ((uint8_t*)(reg)+(field))), (value))


#define FLAG_Z 0b1000  /* zero flag */
#define FLAG_N 0b0100  /* subtraction flag (BCD) */
#define FLAG_H 0b0010  /* half carry flag (BCD) */
#define FLAG_C 0b0001  /* carry flag */

#define READ_R_FLAG(reg, flag) ((READ_R8(reg, REG_F) & flag) ? 1 : 0)
#define SET_R_FLAG(reg, flag) WRITE_R8(reg, REG_F, (READ_R8(reg, REG_F) | flag))
#define CLEAR_R_FLAG(reg, flag) WRITE_R8(reg, REG_F, (READ_R8(reg, REG_F) & ~flag))
#define SET_R_FLAG_VALUE(reg, flag, value) ((value) ? (SET_R_FLAG(reg, flag)) : (CLEAR_R_FLAG(reg, flag)))

void gbc_cpu_init(gbc_cpu_t *cpu);
void gbc_cpu_connect(gbc_cpu_t *cpu, gbc_memory_t *mem);
void gbc_cpu_cycle(gbc_cpu_t *cpu);

#endif
