#pragma once

#include "type_functionality.h"
#include "shared_ptr.h"

/* Type declaration */
typedef shared_ptr vector;

/* Main function's declarations */
vector create_vector(const size_t size, const type_func *const type);
void free_vector(vector const ptr);
int resize_vector(vector const ptr, const size_t new_size);
int append_vector(vector const ptr, const void *const val);
int pop_vector(vector const ptr, void *const val);
int insert_vector(vector const ptr, const size_t index, const void *const val);
int delete_vector(vector const ptr, const size_t index, void *const val);
int set_vector(const vector ptr, const size_t index, void *const val);
int get_vector(const vector ptr, const size_t index, void *const val);
int sort_vector(vector const ptr, int (*compare_func)(const void *, const void *));
int swap_vector(vector const ptr, const size_t index_1, const size_t index_2);
int invert_vector(vector const ptr);
