#include "shared_ptr.h"
#include "util_funcs.h"
#include <stdlib.h>

typedef struct shared_ptr_head
{
    size_t count;
    char * data;
} shared_ptr_head;

shared_ptr malloc_shared_ptr(const size_t data_size, const size_t size)
{
    shared_ptr new_shared_ptr = malloc(sizeof(shared_ptr_head) + size * data_size);
    ((shared_ptr_head*)new_shared_ptr)->count = 1;
    ((shared_ptr_head*)new_shared_ptr)->data = new_shared_ptr + sizeof(shared_ptr_head);
    return new_shared_ptr;
}

shared_ptr calloc_shared_ptr(const size_t data_size, const size_t size)
{
    shared_ptr new_shared_ptr = calloc(1, sizeof(shared_ptr_head) + size * data_size);
    ((shared_ptr_head*)new_shared_ptr)->count = 1;
    ((shared_ptr_head*)new_shared_ptr)->data = new_shared_ptr + sizeof(shared_ptr_head);
    return new_shared_ptr;
}

shared_ptr realloc_shared_ptr(shared_ptr const ptr, const size_t data_size, const size_t size)
{
    shared_ptr new_shared_ptr = calloc(1, sizeof(shared_ptr_head) + size * data_size);
    ((shared_ptr_head*)new_shared_ptr)->data = new_shared_ptr + sizeof(shared_ptr_head);
    return new_shared_ptr;
}

void free_shared_ptr(shared_ptr const ptr)
{
    if (--((shared_ptr_head*)ptr)->count == 0)
        free(ptr);
    return;
}

char *data_shared_ptr(shared_ptr const ptr)
{
    return ((shared_ptr_head*)ptr)->data;
}

shared_ptr copy_shared_ptr(shared_ptr const ptr)
{
    ++((shared_ptr_head*)ptr)->count;
    return ptr;
}
