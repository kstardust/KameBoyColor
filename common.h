#ifndef _COMMON_H
#define _COMMON_H

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "utils.h"

#define DEBUG

#ifndef DEBUG
#define LOG_INFO(fmt, ...) printf("[info]"fmt, ##__VA_ARGS__)
#else
#define LOG_INFO(fmt, ...)
#endif  

#define LOG_ERROR(fmt, ...) printf("[error]"fmt, ##__VA_ARGS__)

#endif