#ifndef _COMMON_H
#define _COMMON_H

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"

#define DEBUG

#ifdef DEBUG
#define LOG_INFO(fmt, ...) printf("[info]"fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) printf("[debug]"fmt, ##__VA_ARGS__)
#else
#define LOG_INFO(fmt, ...)
#define LOG_DEBUG(fmt, ...)
#endif  

#define LOG_ERROR(fmt, ...) printf("[error]"fmt, ##__VA_ARGS__)

#define UINT4_MASK  0xF
#define UINT8_MASK  0xFF
#define UINT16_MASK 0xFFFF

/* https://stackoverflow.com/questions/8868396/game-boy-what-constitutes-a-half-carry */
#define HALF_CARRY_ADD(a, b) (((((a) & UINT4_MASK) + ((b) & UINT4_MASK)) & 0x10) == 0x10)
#define HALF_CARRY_ADC(a, b, c) (((((a) & UINT4_MASK) + ((b) & UINT4_MASK) + ((c) & UINT4_MASK)) & 0x10) == 0x10)

#define HALF_CARRY_ADD_16(a, b) (((((a) & 0xfff) + ((b) & 0xfff)) & 0x1000) == 0x1000)
#define HALF_CARRY_ADC_16(a, b, c) (((((a) & 0xfff) + ((b) & 0xfff) + ((c) & 0xfff)) & 0x1000) == 0x1000)

#define HALF_CARRY_SUB(a, b) ((((a) & UINT4_MASK) < ((b) & UINT4_MASK)) ? 1 : 0)
#define HALF_CARRY_SBC(a, b, c) (((((a)-(c)) & UINT4_MASK) < ((b) & UINT4_MASK)) ? 1 : 0)

#define HALF_CARRY_SUB_16(a, b) ((((a) & 0xfff) < ((b) & 0xfff)) ? 1 : 0)
#define HALF_CARRY_SBC_16(a, b, c) (((((a)-(c)) & 0xfff) < ((b) & 0xfff)) ? 1 : 0)


#endif