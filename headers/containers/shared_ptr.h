#pragma once

#include <stdint.h>

typedef struct shared_ptr_head shared_ptr_head;
typedef shared_ptr_head *shared_ptr;

shared_ptr malloc_shared_ptr(const size_t size, const size_t data_size);
shared_ptr calloc_shared_ptr(const size_t size, const size_t data_size);
shared_ptr realloc_shared_ptr(shared_ptr const ptr, const size_t size, const size_t data_size);
void free_shared_ptr(shared_ptr const ptr);
size_t count_shared_ptr(shared_ptr const ptr);
void *data_shared_ptr(shared_ptr const ptr);
shared_ptr copy_shared_ptr(shared_ptr const ptr);
