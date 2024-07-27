/* TODO: cross-platform  */

#include <stdlib.h>
#include <time.h>
#include "utils.h"
#include <stdlib.h>

void* 
malloc_memory(size_t size)
{
    return (void*)malloc(size);
}

void 
free_memory(void *ptr)
{
    free(ptr);        
}

uint64_t
get_time()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
}