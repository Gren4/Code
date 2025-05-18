#pragma once

#include <stdint.h>

typedef struct vector vector;

vector *create_vector(const size_t size, const size_t data_size);
void free_vector(vector *const v);
void resize_vector(vector *const v, const size_t new_size);
void append_vector(vector *const v, const void *const val);
int pop_vector(vector *const v, void *const val);
int insert_vector(vector *const v, const size_t index, const void *const val);
int delete_vector(vector *const v, const size_t index, void *const val);
void *at_vector(const vector *const v, const size_t index);
void sort_vector(vector *const v, int (*compare_func)(const void *, const void *));
void swap_vector(vector *const v, const size_t index_1, const size_t index_2);
void invert_vector(vector *const v);
size_t find_vector(const vector *const v, const void *const val);
