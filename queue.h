#pragma once

#include "type_functionality.h"

typedef struct queue queue;

queue *create_queue(const size_t size, const type_func *const type);
void free_queue(queue *const q);
int push_queue(queue *const q, const void* const val);
int pop_queue(queue *const q, void* const val);
