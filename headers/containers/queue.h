#pragma once

#include "type_functionality.h"
#include "shared_ptr.h"

/* Type declaration */
typedef shared_ptr queue;

/* Main function's declarations */
queue create_queue(const size_t size, const type_func *const type);
size_t count_queue(const queue q);
void free_queue(queue const q);
int push_queue(queue const q, const void* const val);
int pop_queue(queue const q, void* const val);
int at_front_queue(queue const q, void* const val);
