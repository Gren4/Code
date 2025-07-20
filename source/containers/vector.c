#include "vector.h"
#include "util_funcs.h"
#include <stdlib.h>
#include <string.h>

#define VECTOR_MIN_SIZE 16

/* Struct declaration */
typedef struct vector_t
{
    size_t count;
    size_t size;
    const type_func *type;
    void *container;
} vector_t;

/* Static function's declarations */
static inline int expand_vector(vector_t *const v);
static inline int shrink_vector(vector_t *const v);

/* Main functions */
vector create_vector(const size_t size, const type_func *const type)
{
    vector ptr = malloc_shared_ptr(1, sizeof(vector_t));
    if (ptr == NULL)
        return NULL;
    size_t mul_of_2_size = size < VECTOR_MIN_SIZE ? VECTOR_MIN_SIZE : next_power_of_2(size);
    vector_t new_v = {
        .count = 0,
        .size = mul_of_2_size,
        .type = type,
        .container = calloc(mul_of_2_size, type->t_size)};
    if (new_v.container == NULL)
    {
        free_shared_ptr(ptr);
        return NULL;
    }
    memcpy(data_shared_ptr(ptr), &new_v, sizeof(vector_t));
    return ptr;
}

void free_vector(vector const ptr)
{
    if (ptr == NULL)
        return;
    if (count_shared_ptr(ptr) == 1)
    {
        vector_t *v = data_shared_ptr(ptr);
        if (v->container != NULL)
        {
            size_t i = 0;
            for (; i < v->count; i++)
            {
                v->type->t_free(v->type->t_at(v->container, i));
            }
            free(v->container);
        }
        v->count = 0;
        v->size = 0;
        v->container = NULL;
    }
    free_shared_ptr(ptr);
    return;
}

int resize_vector(vector const ptr, const size_t new_size)
{
    if (ptr == NULL)
        return 0;
    vector_t *v = data_shared_ptr(ptr);
    if (v->size == new_size)
        return 1;
    size_t mul_of_2_size = next_power_of_2(new_size);
    if (mul_of_2_size < v->count)
    {
        size_t i = v->count - 1;
        for (; i >= mul_of_2_size; i++)
        {
            v->type->t_free(v->type->t_at(v->container, i));
        }
    }
    void *new_container = realloc(v->container, mul_of_2_size * v->type->t_size);
    if (new_container == NULL)
        return 0;
    memset(v->type->t_at(new_container, v->size), 0, (mul_of_2_size - v->size) * v->type->t_size);
    v->container = new_container;
    v->size = mul_of_2_size;
    v->count = new_size;
    return 1;
}

int append_vector(vector const ptr, const void *const val)
{
    if (ptr == NULL)
        return 0;
    vector_t *v = data_shared_ptr(ptr);
    if (val == NULL || v->count == SIZE_MAX || expand_vector(v) == 0)
        return 0;
    v->type->t_cpy(v->type->t_at(v->container, v->count - 1), val);
    return 1;
}

int pop_vector(vector const ptr, void *const val)
{
    if (ptr == NULL)
        return 0;
    vector_t *v = data_shared_ptr(ptr);
    if (v->count == 0)
        return 0;
    void *container_val = v->type->t_at(v->container, v->count - 1);
    if (val != NULL)
        v->type->t_cpy(val, container_val);
    v->type->t_free(container_val);
    return shrink_vector(v);
}

int insert_vector(vector const ptr, const size_t index, const void *const val)
{
    if (ptr == NULL)
        return 0;
    vector_t *v = data_shared_ptr(ptr);
    if (val == NULL || v->count == SIZE_MAX || index > v->count || expand_vector(v) == 0)
        return 0;
    void *container_val = v->type->t_at(v->container, index);
    memcpy(v->type->t_at(v->container, index + 1), container_val, (v->count - index - 1) * v->type->t_size);
    v->type->t_cpy(container_val, val);
    return 1;
}

int delete_vector(vector const ptr, const size_t index, void *const val)
{
    if (ptr == NULL)
        return 0;
    vector_t *v = data_shared_ptr(ptr);
    if (v->count == 0 || index >= v->count)
        return 0;
    void *container_val = v->type->t_at(v->container, index);
    if (val != NULL)
        v->type->t_cpy(val, container_val);
    v->type->t_free(container_val);
    memcpy(container_val, v->type->t_at(v->container, index + 1), (v->count - index - 1) * v->type->t_size);
    return shrink_vector(v);
}

int set_vector(const vector ptr, const size_t index, void *const val)
{
    if (ptr == NULL)
        return 0;
    vector_t *v = data_shared_ptr(ptr);
    if (val == NULL || index >= v->count)
        return 0;
    v->type->t_free(v->type->t_at(v->container, index));
    v->type->t_cpy(v->type->t_at(v->container, index), val);
    return 1;
}

int get_vector(const vector ptr, const size_t index, void *const val)
{
    if (ptr == NULL)
        return 0;
    vector_t *v = data_shared_ptr(ptr);
    if (val == NULL || index >= v->count)
        return 0;
    v->type->t_cpy(val, v->type->t_at(v->container, index));
    return 1;
}

int sort_vector(vector const ptr, int (*compare_func)(const void *, const void *))
{
    if (ptr == NULL)
        return 0;
    vector_t *v = data_shared_ptr(ptr);
    qsort(v->container, v->count, v->type->t_size, compare_func);
    return 1;
}

int swap_vector(vector const ptr, const size_t index_1, const size_t index_2)
{
    if (ptr == NULL)
        return 0;
    vector_t *v = data_shared_ptr(ptr);
    if (index_1 >= v->count && index_2 >= v->count)
        return 0;
    v->type->t_swap(v->type->t_at(v->container, index_1), v->type->t_at(v->container, index_2));
    return 1;
}

int invert_vector(vector const ptr)
{
    if (ptr == NULL)
        return 0;
    vector_t *v = data_shared_ptr(ptr);
    if (v->count <= 1)
        return 1;
    size_t i = 0;
    size_t j = v->count - 1;
    for (; j > i; i++, j--)
    {
        v->type->t_swap(v->type->t_at(v->container, i), v->type->t_at(v->container, j));
    }
    return 1;
}

/* Static functions */
static inline int expand_vector(vector_t *const v)
{
    if (++v->count <= v->size)
        return 1;
    size_t mul_of_2_size = next_power_of_2(v->count);
    void *new_container = realloc(v->container, mul_of_2_size * v->type->t_size);
    if (new_container == NULL)
        return 0;
    memset(v->type->t_at(new_container, v->size), 0, (mul_of_2_size - v->size) * v->type->t_size);
    v->container = new_container;
    v->size = mul_of_2_size;
    return 1;
}

static inline int shrink_vector(vector_t *const v)
{
    if (--v->count > v->size >> 3)
        return 1;
    size_t mul_of_2_size = next_power_of_2(v->count);
    if (mul_of_2_size <= VECTOR_MIN_SIZE)
        return 1;
    void *new_container = realloc(v->container, mul_of_2_size * v->type->t_size);
    if (new_container == NULL)
        return 0;
    v->container = new_container;
    v->size = mul_of_2_size;
    return 1;
}

/* Type functionality */
void *t_at_vector(const void *const src, const size_t index)
{
    return ((vector *)src) + index;
}

int t_cmp_vector(const void *const src_1, const void *const src_2)
{
    vector_t *a = data_shared_ptr(*(vector *)src_1);
    vector_t *b = data_shared_ptr(*(vector *)src_2);
    return a->size > b->size;
}

void *t_cpy_vector(void *const dest, const void *const src)
{
    if (*(vector *)dest != *(vector *)src)
        *(vector *)dest = copy_shared_ptr(*(vector *)src);
    return dest;
}

void *t_move_vector(void *const dest, const void *const src)
{
    *(vector *)dest = *(vector *)src;
    return dest;
}

void t_free_vector(void *const src)
{
    vector *val = (vector *)src;
    free_vector(*val);
    return;
}

size_t t_hash_vector(const void *const src)
{
    vector_t *v = data_shared_ptr(*(vector *)src);
    size_t h = 0;
    h = hash_size_t(v->count);
    h = hash_size_t(v->size) ^ (h << 6) ^ (h >> 2);
    h = hash_size_t((size_t)v->type) ^ (h << 6) ^ (h >> 2);
    h = hash_size_t((size_t)v->container) ^ (h << 6) ^ (h >> 2);
    return h;
}

void t_swap_vector(void *const src_1, void *const src_2)
{
    if (src_1 == src_2)
        return;
    size_t *a = (size_t *)src_1;
    size_t *b = (size_t *)src_2;
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
    return;
}

const type_func orig_f_vector = {
    .t_size = sizeof(vector),
    .t_at = t_at_vector,
    .t_cmp = t_cmp_vector,
    .t_cpy = t_cpy_vector,
    .t_move = t_move_vector,
    .t_free = t_free_vector,
    .t_hash = t_hash_vector,
    .t_swap = t_swap_vector};

const type_func *f_vector = &orig_f_vector;
