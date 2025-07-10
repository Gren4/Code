#include "vector.h"
#include "util_funcs.h"
#include <stdlib.h>
#include <string.h>

typedef struct vector
{
    size_t count;
    size_t size;
    const type_func *type;
    char *data;
} vector;

vector *create_vector(const size_t size, const type_func *const type)
{
    size_t mul_of_2_size = next_power_of_2(size);
    vector new_v = {
        .count = 0,
        .size = mul_of_2_size,
        .type = type,
        .data = (char *)calloc(mul_of_2_size, type->t_size)};
    if (new_v.data == NULL)
        return NULL;
    return (vector *)memcpy(malloc(sizeof(vector)), &new_v, sizeof(vector));
}

void free_vector(vector *const v)
{
    if (v->data != NULL)
    {
        if (v->type->t_free != NULL && v->count > 0)
        {
            ssize_t i = 0;
            for (; i < v->count; i++)
            {
                v->type->t_free(v->type->t_at(v->data, i));
            }
        }
        free(v->data);
    }
    v->count = 0;
    v->size = 0;
    v->data = NULL;
    return;
}

int resize_vector(vector *const v, const size_t new_size)
{
    if (v->size == new_size)
        return 1;
    size_t mul_of_2_size = next_power_of_2(new_size);
    char *new_data = realloc(v->data, mul_of_2_size * v->type->t_size);
    if (new_data == NULL)
        return 0;
    memset(new_data + v->size * v->type->t_size, 0, (mul_of_2_size - v->size) * v->type->t_size);
    v->data = new_data;
    v->size = mul_of_2_size;
    v->count = new_size;
    return 1;
}

static inline int expand_vector(vector *const v)
{
    if (++v->count <= v->size)
        return 1;
    size_t mul_of_2_size = next_power_of_2(v->count);
    char *new_data = realloc(v->data, mul_of_2_size * v->type->t_size);
    if (new_data == NULL)
        return 0;
    memset(new_data + v->size * v->type->t_size, 0, (mul_of_2_size - v->size) * v->type->t_size);
    v->data = new_data;
    v->size = mul_of_2_size;
    return 1;
}

static inline int shrink_vector(vector *const v)
{
    if (v->count <= 0)
        return 0;
    if (--v->count > v->size >> 3)
        return 1;
    size_t mul_of_2_size = next_power_of_2(v->count);
    char *new_data = realloc(v->data, mul_of_2_size * v->type->t_size);
    if (new_data == NULL)
        return 0;
    v->data = new_data;
    v->size = mul_of_2_size;
    return 1;
}

int append_vector(vector *const v, const void *const val)
{
    if (expand_vector(v) == 0)
        return 0;
    v->type->t_cpy(v->type->t_at(v->data, v->count - 1), val);
    return 1;
}

int pop_vector(vector *const v, void *const val)
{
    if (v->count <= 0)
        return 0;
    char *ptr = v->type->t_at(v->data, v->count - 1);
    if (val != NULL)
        v->type->t_cpy(val, ptr);
    if (v->type->t_free != NULL)
        v->type->t_free(ptr);
    return shrink_vector(v);
}

int insert_vector(vector *const v, const size_t index, const void *const val)
{
    if (index > v->count)
        return 0;
    if (expand_vector(v) == 0)
        return 0;
    char *ptr = v->type->t_at(v->data, index);
    memcpy(v->type->t_at(v->data, index + 1), ptr, (v->count - index - 1) * v->type->t_size);
    v->type->t_cpy(ptr, val);
    return 1;
}

int delete_vector(vector *const v, const size_t index, void *const val)
{
    if (index >= v->count)
        return 0;
    char *ptr = v->type->t_at(v->data, index);
    if (val != NULL)
        v->type->t_cpy(val, ptr);
    if (v->type->t_free != NULL)
        v->type->t_free(ptr);
    memcpy(ptr, v->type->t_at(v->data, index + 1), (v->count - index - 1) * v->type->t_size);
    return shrink_vector(v);
}

inline void *at_vector(const vector *const v, const size_t index)
{
    return v->type->t_at(v->data, index);
}

void sort_vector(vector *const v, int (*compare_func)(const void *, const void *))
{
    qsort(v->data, v->count, v->type->t_size, compare_func);
    return;
}

int swap_vector(vector *const v, const size_t index_1, const size_t index_2)
{
    if (index_1 >= v->count && index_2 >= v->count)
        return 0;
    v->type->t_swap(v->type->t_at(v->data, index_1), v->type->t_at(v->data, index_2));
    return 1;
}

void invert_vector(vector *const v)
{
    ssize_t i = 0;
    ssize_t j = v->count - 1;
    for (; j > i; i++, j--)
    {
        swap_vector(v, i, j);
    }
    return;
}
