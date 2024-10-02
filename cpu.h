#ifndef _CPU_H
#define _CPU_H

#include <stdint.h>
#include "memory.h"

typedef struct cpu_register cpu_register_t;
typedef struct gbc_cpu gbc_cpu_t;

#define CLOCK_RATE 4194304                        /* 4.194304 MHz */
#define CLOCK_CYCLE (1000000000 / CLOCK_RATE)     /* nanoseconds */
#define FRAME_PER_SECOND 60
#define LOGIC_FRAME_RATE 60
#define FRAME_INTERVAL (1000000000 / LOGIC_FRAME_RATE)
#define CYCLES_PER_FRAME (CLOCK_RATE / FRAME_PER_SECOND)


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
    uint8_t *ifp;           /* interrupt flag 'pointer'(it is a pointer to io port) */

    uint64_t cycles;
    uint16_t ins_cycles;   /* current instruction cost */
    uint8_t ime;           /* interrupt master enable */
    uint8_t ier;           /* interrupt enable register */
    uint8_t ime_insts:4;   /* instruction count to set ime */
    uint8_t halt:2;        /* halt state */
    uint8_t dspeed:1;      /* doublespeed state */
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


#define FLAG_Z 0b10000000  /* zero flag */
#define FLAG_N 0b01000000  /* subtraction flag (BCD) */
#define FLAG_H 0b00100000  /* half carry flag (BCD) */
#define FLAG_C 0b00010000  /* carry flag */

#define READ_R_FLAG(reg, flag) ((READ_R8(reg, REG_F) & flag) ? 1 : 0)
#define SET_R_FLAG(reg, flag) WRITE_R8(reg, REG_F, (READ_R8(reg, REG_F) | flag))
#define CLEAR_R_FLAG(reg, flag) WRITE_R8(reg, REG_F, (READ_R8(reg, REG_F) & ~flag))
#define SET_R_FLAG_VALUE(reg, flag, value) ((value) ? (SET_R_FLAG(reg, flag)) : (CLEAR_R_FLAG(reg, flag)))

#define INTERRUPT_VBLANK   0x1
#define INTERRUPT_LCD_STAT 0x2
#define INTERRUPT_TIMER    0x4
#define INTERRUPT_SERIAL   0x8
#define INTERRUPT_JOYPAD   0x10
#define INTERRUPT_MASK     0x1F
#define CPU_REQUEST_INTERRUPT(cpu, flag) (*(cpu)->ifp |= (flag))

#define INT_HANDLER_VBLANK   0x40
#define INT_HANDLER_LCD_STAT 0x48
#define INT_HANDLER_TIMER    0x50
#define INT_HANDLER_SERIAL   0x58
#define INT_HANDLER_JOYPAD   0x60

/* https://gbdev.io/pandocs/CGB_Registers.html#ff4d--key1-cgb-mode-only-prepare-speed-switch */
#define KEY1_CPU_SWITCH_ARMED 0x1
#define KEY1_CPU_CURRENT_MODE 0x80

void gbc_cpu_init(gbc_cpu_t *cpu);
void gbc_cpu_connect(gbc_cpu_t *cpu, gbc_memory_t *mem);
void gbc_cpu_cycle(gbc_cpu_t *cpu);

/* ORDER: "PC", "SP", "A", "F", "B", "C", "D", "E", "H", "L", "Z", "N", "H", "C", "IME", "IE", "IF" */
#define DEBUG_CPU_REGISTERS_SIZE 17
void debug_get_all_registers(gbc_cpu_t *cpu, int values[DEBUG_CPU_REGISTERS_SIZE]);

#endif
