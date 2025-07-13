#include "vector.h"
#include "shared_ptr.h"
#include "util_funcs.h"
#include <stdlib.h>
#include <string.h>

#define VECTOR_MIN_SIZE 16

typedef struct vector
{
    size_t count;
    size_t size;
    const type_func *type;
    shared_ptr container;
} vector;

vector *create_vector(const size_t size, const type_func *const type)
{
    size_t mul_of_2_size = size < VECTOR_MIN_SIZE ? VECTOR_MIN_SIZE : next_power_of_2(size);
    vector new_v = {
        .count = 0,
        .size = mul_of_2_size,
        .type = type,
        .container = calloc_shared_ptr(mul_of_2_size, type->t_size)};
    if (new_v.container == NULL)
        return NULL;
    return (vector *)memcpy(malloc(sizeof(vector)), &new_v, sizeof(vector));
}

void free_vector(vector *const v)
{
    if (v->container != NULL)
    {
        ssize_t i = 0;
        char *container_data = data_shared_ptr(v->container);
        for (; i < v->count; i++)
        {
            v->type->t_free(v->type->t_at(container_data, i));
        }
        free_shared_ptr(v->container);
    }
    v->count = 0;
    v->size = 0;
    v->container = NULL;
    return;
}

int resize_vector(vector *const v, const size_t new_size)
{
    if (v->size >= new_size)
        return 1;
    size_t mul_of_2_size = next_power_of_2(new_size);
    shared_ptr new_container = realloc_shared_ptr(v->container, mul_of_2_size, v->type->t_size);
    if (new_container == NULL)
        return 0;
    memset(v->type->t_at(data_shared_ptr(new_container), v->size), 0, (mul_of_2_size - v->size) * v->type->t_size);
    v->container = new_container;
    v->size = mul_of_2_size;
    v->count = new_size;
    return 1;
}

static inline int expand_vector(vector *const v)
{
    if (++v->count <= v->size)
        return 1;
    size_t mul_of_2_size = next_power_of_2(v->count);
    shared_ptr new_container = realloc_shared_ptr(v->container, mul_of_2_size, v->type->t_size);
    if (new_container == NULL)
        return 0;
    memset(v->type->t_at(data_shared_ptr(new_container), v->size), 0, (mul_of_2_size - v->size) * v->type->t_size);
    v->container = new_container;
    v->size = mul_of_2_size;
    return 1;
}

static inline int shrink_vector(vector *const v)
{
    if (--v->count > v->size >> 3)
        return 1;
    size_t mul_of_2_size = next_power_of_2(v->count);
    if (mul_of_2_size <= VECTOR_MIN_SIZE)
        return 1;
    shared_ptr new_container = realloc_shared_ptr(v->container, mul_of_2_size, v->type->t_size);
    if (new_container == NULL)
        return 0;
    v->container = new_container;
    v->size = mul_of_2_size;
    return 1;
}

int append_vector(vector *const v, const void *const val)
{
    if (val == NULL || v->count == SIZE_MAX || expand_vector(v) == 0)
        return 0;
    v->type->t_cpy(v->type->t_at(data_shared_ptr(v->container), v->count - 1), val);
    return 1;
}

int pop_vector(vector *const v, void *const val)
{
    if (v->count == 0)
        return 0;
    char *ptr = v->type->t_at(data_shared_ptr(v->container), v->count - 1);
    if (val != NULL)
        v->type->t_cpy(val, ptr);
    v->type->t_free(ptr);
    return shrink_vector(v);
}

int insert_vector(vector *const v, const size_t index, const void *const val)
{
    if (val == NULL || v->count == SIZE_MAX || index > v->count || expand_vector(v) == 0)
        return 0;
    char *container_data = data_shared_ptr(v->container);
    char *ptr = v->type->t_at(container_data, index);
    memcpy(v->type->t_at(container_data, index + 1), ptr, (v->count - index - 1) * v->type->t_size);
    v->type->t_cpy(ptr, val);
    return 1;
}

int delete_vector(vector *const v, const size_t index, void *const val)
{
    if (v->count == 0 || index >= v->count)
        return 0;
    char *container_data = data_shared_ptr(v->container);
    char *ptr = v->type->t_at(container_data, index);
    if (val != NULL)
        v->type->t_cpy(val, ptr);
    v->type->t_free(ptr);
    memcpy(ptr, v->type->t_at(container_data, index + 1), (v->count - index - 1) * v->type->t_size);
    return shrink_vector(v);
}

int set_vector(const vector *const v, const size_t index, void *const val)
{
    if (val == NULL || index >= v->count)
        return 0;
    char *container_data = data_shared_ptr(v->container);
    v->type->t_free(v->type->t_at(container_data, index));
    v->type->t_cpy(v->type->t_at(container_data, index), val);
    return 1;
}

int get_vector(const vector *const v, const size_t index, void *const val)
{
    if (val == NULL || index >= v->count)
        return 0;
    v->type->t_cpy(val, v->type->t_at(data_shared_ptr(v->container), index));
    return 1;
}

void sort_vector(vector *const v, int (*compare_func)(const void *, const void *))
{
    qsort(data_shared_ptr(v->container), v->count, v->type->t_size, compare_func);
    return;
}

int swap_vector(vector *const v, const size_t index_1, const size_t index_2)
{
    if (index_1 >= v->count && index_2 >= v->count)
        return 0;
    char *container_data = data_shared_ptr(v->container);
    v->type->t_swap(v->type->t_at(container_data, index_1), v->type->t_at(container_data, index_2));
    return 1;
}

void invert_vector(vector *const v)
{
    if (v->count <= 1)
        return;
    ssize_t i = 0;
    ssize_t j = v->count - 1;
    for (; j > i; i++, j--)
    {
        swap_vector(v, i, j);
    }
    return;
}
