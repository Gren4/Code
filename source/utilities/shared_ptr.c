#include "shared_ptr.h"
#include <stdlib.h>

typedef struct shared_ptr_head
{
    size_t count;
    void *data;
} shared_ptr_head;

shared_ptr malloc_shared_ptr(const size_t size, const size_t data_size)
{
    shared_ptr new_shared_ptr = malloc(sizeof(shared_ptr_head));
    if (new_shared_ptr == NULL)
        return NULL;
    void *data = malloc(size * data_size);
    if (data == NULL)
    {
        free(new_shared_ptr);
        return NULL;
    }
    new_shared_ptr->count = 1;
    new_shared_ptr->data = data;
    return new_shared_ptr;
}

shared_ptr calloc_shared_ptr(const size_t size, const size_t data_size)
{
    shared_ptr new_shared_ptr = malloc(sizeof(shared_ptr_head));
    if (new_shared_ptr == NULL)
        return NULL;
    void *data = calloc(size, data_size);
    if (data == NULL)
    {
        free(new_shared_ptr);
        return NULL;
    }
    new_shared_ptr->count = 1;
    new_shared_ptr->data = data;
    return new_shared_ptr;
}

shared_ptr realloc_shared_ptr(shared_ptr const ptr, const size_t size, const size_t data_size)
{
    if (ptr == NULL)
        return NULL;
    void *new_data = realloc(ptr->data, size * data_size);
    if (new_data == NULL)
    {
        free(ptr);
        return NULL;
    }
    ptr->data = new_data;
    return ptr;
}

void free_shared_ptr(shared_ptr const ptr)
{
    if (ptr != NULL && --ptr->count == 0)
    {
        if (ptr->data != NULL)
            free(ptr->data);
        ptr->data = NULL;
        free(ptr);
    }
    return;
}

size_t count_shared_ptr(shared_ptr const ptr)
{
    if (ptr != NULL)
        return ptr->count;
    else
        return 0;
}

void *data_shared_ptr(shared_ptr const ptr)
{
    if (ptr != NULL)
        return ptr->data;
    else
        return NULL;
}

shared_ptr copy_shared_ptr(shared_ptr const ptr)
{
    if (ptr == NULL)
        return NULL;
    ++ptr->count;
    return ptr;
}
