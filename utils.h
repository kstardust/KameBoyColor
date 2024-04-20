#ifndef _UTILS_H
#define _UTILS_H

#include <stdint.h>

void *malloc_memory(size_t size);
void free_memory(void *ptr);

/* 
    Return the time in nanoseconds, it neither represents the current time nor the time since the program started,
    should only be used to measure the interval.
*/
uint64_t get_time(); 

#endif
