/* TODO: cross-platform  */

#include <stdlib.h>
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
