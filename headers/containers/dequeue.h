#pragma once

#include "type_functionality.h"
#include "shared_ptr.h"

/* Type declaration */
typedef shared_ptr dequeue;

/* Main function's declarations */
dequeue create_dequeue(const size_t size, const type_func *const type);
size_t count_dequeue(const dequeue dq);
void free_dequeue(dequeue const dq);
int push_front_dequeue(dequeue const dq, const void *const val);
int push_back_dequeue(dequeue const dq, const void *const val);
int pop_front_dequeue(dequeue const dq, void *const val);
int pop_back_dequeue(dequeue const dq, void *const val);
int at_front_dequeue(dequeue const dq, void *const val);
int at_back_dequeue(dequeue const dq, void *const val);
