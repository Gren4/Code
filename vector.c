#include "vector.h"
#include <stdlib.h>
#include <string.h>
#include "util_funcs.h"

vector create_vector(const size_t size, const size_t data_size)
{
    size_t mul_of_2_size = next_power_of_2(size);
    vector new_vector = {
        .data_size = data_size,
        .count = 0,
        .size = mul_of_2_size,
        .data = (void *)calloc(mul_of_2_size, data_size)};
    return new_vector;
}

void free_vector(vector *const v)
{
    if (v->data != nullptr)
        free(v->data);
    v->data_size = 0;
    v->count = 0;
    v->size = 0;
    v->data = nullptr;
    return;
}

void resize_vector(vector *const v, const size_t new_size)
{
    if (v->size == new_size)
        return;
    size_t mul_of_2_size = next_power_of_2(new_size);
    v->data = realloc(v->data, mul_of_2_size * v->data_size);
    v->size = mul_of_2_size;
    v->count = new_size;
    return;
}

static inline void expand_vector(vector *const v)
{
    if (++v->count <= v->size)
        return;
    size_t mul_of_2_size = next_power_of_2(v->count);
    v->data = realloc(v->data, mul_of_2_size * v->data_size);
    v->size = mul_of_2_size;
    return;
}

static inline int shrink_vector(vector *const v)
{
    if (v->count <= 0)
        return 0;
    if (--v->count >= v->size >> 3)
        return 1;
    size_t mul_of_2_size = next_power_of_2(v->count);
    v->data = realloc(v->data, mul_of_2_size * v->data_size);
    v->size = mul_of_2_size;
    return 1;
}

void append_vector(vector *const v, const void *const val)
{
    expand_vector(v);
    memcpy(v->data + (v->count - 1) * v->data_size, val, v->data_size);
    return;
}

int pop_vector(vector *const v, void *const val)
{
    if (val != nullptr)
        memcpy(val, v->data + (v->count - 1) * v->data_size, v->data_size);
    return shrink_vector(v);
}

int insert_vector(vector *const v, const size_t index, void *const val)
{
    if (index > v->count)
        return 0;
    expand_vector(v);
    memcpy(v->data + (index + 1) * v->data_size, v->data + index * v->data_size, (v->count - index - 1) * v->data_size);
    memcpy(v->data + index * v->data_size, val, v->data_size);
    return 1;
}

int delete_vector(vector *const v, const size_t index, void *const val)
{
    if (index >= v->count)
        return 0;
    if (val != nullptr)
        memcpy(val, v->data + index * v->data_size, v->data_size);
    memcpy(v->data + index * v->data_size, v->data + (index + 1) * v->data_size, (v->count - index - 1) * v->data_size);
    return shrink_vector(v);
}

inline void *at_vector(const vector *const v, const size_t index)
{
    return v->data + index * v->data_size;
}

void sort_vector(vector *const v, int (*compare_func)(const void *, const void *))
{
    qsort(v->data, v->count, v->data_size, compare_func);
    return;
}

void swap_vector(vector *const v, const size_t index_1, const size_t index_2)
{
    size_t i = 0;
    uint8_t *a = (uint8_t *)at_vector(v, index_1);
    uint8_t *b = (uint8_t *)at_vector(v, index_2);
    for (; i < v->data_size; i++, a++, b++)
    {
        *a ^= *b;
        *b ^= *a;
        *a ^= *b;
    }
    return;
}

void invert_vector(vector *const v)
{
    size_t i = 0;
    size_t j = v->count - 1;
    for (; j > i; i++, j--)
    {
        swap_vector(v, i, j);
    }
    return;
}

size_t find_vector(const vector *const v, const void *const val)
{
    size_t i = 0;
    for (; i < v->count; i++)
    {
        if (memcmp(at_vector(v, i), val, v->data_size) == 0)
            return i;
    }
    return v->count;
}
