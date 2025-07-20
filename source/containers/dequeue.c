#include "dequeue.h"
#include "util_funcs.h"
#include <stdlib.h>
#include <string.h>

#define DEQUEUE_MIN_SIZE 16

/* Struct declaration */
typedef struct dequeue_t
{
    size_t count;
    size_t size;
    size_t front;
    size_t back;
    const type_func *type;
    void *container;
} dequeue_t;

/* Static function's declarations */
static int expand_dequeue(dequeue_t *const dq);
static int shrink_dequeue(dequeue_t *const dq);

/* Main functions */
dequeue create_dequeue(const size_t size, const type_func *const type)
{
    dequeue ptr = malloc_shared_ptr(1, sizeof(dequeue_t));
    if (ptr == NULL)
        return NULL;
    size_t mul_of_2_size = size < DEQUEUE_MIN_SIZE ? DEQUEUE_MIN_SIZE : next_power_of_2(size);
    dequeue_t new_dq = {
        .count = 0,
        .size = mul_of_2_size,
        .front = 0,
        .back = 0,
        .type = type,
        .container = calloc(mul_of_2_size, type->t_size)};
    if (new_dq.container == NULL)
    {
        free_shared_ptr(ptr);
        return NULL;
    }
    memcpy(data_shared_ptr(ptr), &new_dq, sizeof(dequeue_t));
    return ptr;
}

size_t count_dequeue(const dequeue ptr)
{
    if (ptr == NULL)
        return 0;
    dequeue_t *dq = data_shared_ptr(ptr);
    return dq->count;
}

void free_dequeue(dequeue const ptr)
{
    if (ptr == NULL)
        return;
    if (count_shared_ptr(ptr) == 1)
    {
        dequeue_t *dq = data_shared_ptr(ptr);
        if (dq->container != NULL)
        {
            size_t i = 0;
            for (; i < dq->count; i++)
            {
                dq->type->t_free(dq->type->t_at(dq->container, i));
            }
            free(dq->container);
        }
        dq->count = 0;
        dq->size = 0;
        dq->front = 0;
        dq->back = 0;
        dq->container = NULL;
    }
    free_shared_ptr(ptr);
    return;
}

int push_front_dequeue(dequeue const ptr, const void *const val)
{
    if (ptr == NULL)
        return 0;
    dequeue_t *dq = data_shared_ptr(ptr);
    if (val == NULL || dq->count == SIZE_MAX || expand_dequeue(dq) == 0)
        return 0;
    if (dq->front != dq->back || dq->count > 1)
        dq->front = dq->front == 0 ? dq->size - 1 : dq->front - 1;
    dq->type->t_cpy(dq->type->t_at(dq->container, dq->front), val);
    return 1;
}

int push_back_dequeue(dequeue const ptr, const void *const val)
{
    if (ptr == NULL)
        return 0;
    dequeue_t *dq = data_shared_ptr(ptr);
    if (val == NULL || dq->count == SIZE_MAX || expand_dequeue(dq) == 0)
        return 0;
    if (dq->back != dq->front || dq->count > 1)
        dq->back = (dq->back + 1) % dq->size;
    dq->type->t_cpy(dq->type->t_at(dq->container, dq->back), val);
    return 1;
}

int pop_front_dequeue(dequeue const ptr, void *const val)
{
    if (ptr == NULL)
        return 0;
    dequeue_t *dq = data_shared_ptr(ptr);
    if (dq->count == 0)
        return 0;
    void *container_val = dq->type->t_at(dq->container, dq->front);
    if (val != NULL)
        dq->type->t_cpy(val, container_val);
    dq->type->t_free(container_val);
    dq->front = (dq->front + 1) % dq->size;
    return shrink_dequeue(dq);
}

int pop_back_dequeue(dequeue const ptr, void *const val)
{
    if (ptr == NULL)
        return 0;
    dequeue_t *dq = data_shared_ptr(ptr);
    if (dq->count == 0)
        return 0;
    void *container_val = dq->type->t_at(dq->container, dq->back);
    if (val != NULL)
        dq->type->t_cpy(val, container_val);
    dq->type->t_free(container_val);
    dq->back = dq->back == 0 ? dq->size - 1 : dq->back - 1;
    return shrink_dequeue(dq);
}

int at_front_dequeue(dequeue const ptr, void *const val)
{
    if (ptr == NULL)
        return 0;
    dequeue_t *dq = data_shared_ptr(ptr);
    if (val == NULL || dq->count == 0)
        return 0;
    dq->type->t_cpy(val, dq->type->t_at(dq->container, dq->front));
    return 1;
}

int at_back_dequeue(dequeue const ptr, void *const val)
{
    if (ptr == NULL)
        return 0;
    dequeue_t *dq = data_shared_ptr(ptr);
    if (val == NULL || dq->count == 0)
        return 0;
    dq->type->t_cpy(val, dq->type->t_at(dq->container, dq->back));
    return 1;
}

/* Static functions */
static int expand_dequeue(dequeue_t *const dq)
{
    if (++dq->count <= dq->size)
        return 1;
    size_t mul_of_2_size = next_power_of_2(dq->count);
    void *new_container = realloc(dq->container, mul_of_2_size * dq->type->t_size);
    if (new_container == NULL)
        return 0;
    if (dq->front != 0)
    {
        size_t old_count = dq->count - 1;
        size_t start_offset = dq->back + 1;
        memcpy(dq->type->t_at(new_container, old_count), new_container, start_offset * dq->type->t_size);
        memcpy(new_container, dq->type->t_at(new_container, start_offset), old_count * dq->type->t_size);
        dq->front = 0;
        dq->back = old_count - 1;
    }
    memset(dq->type->t_at(new_container, dq->size), 0, (mul_of_2_size - dq->size) * dq->type->t_size);
    dq->container = new_container;
    dq->size = mul_of_2_size;
    return 1;
}

static int shrink_dequeue(dequeue_t *const dq)
{
    if (--dq->count > dq->size >> 3)
        return 1;
    if (dq->count == 0)
    {
        dq->front = 0;
        dq->back = 0;
    }
    size_t mul_of_2_size = next_power_of_2(dq->count);
    if (mul_of_2_size <= DEQUEUE_MIN_SIZE)
        return 1;
    if (dq->front != 0)
    {
        if (dq->back > dq->front)
        {
            memcpy(dq->container, dq->type->t_at(dq->container, dq->front), dq->count * dq->type->t_size);
        }
        else
        {
            size_t start_offset = dq->back + 1;
            size_t end_size = dq->count - start_offset;
            memcpy(dq->type->t_at(dq->container, start_offset), dq->type->t_at(dq->container, dq->front), end_size * dq->type->t_size);
            memcpy(dq->type->t_at(dq->container, dq->count), dq->container, start_offset * dq->type->t_size);
            memcpy(dq->container, dq->type->t_at(dq->container, start_offset), dq->count * dq->type->t_size);
        }
        dq->front = 0;
        dq->back = dq->count - 1;
    }
    void *new_container = realloc(dq->container, mul_of_2_size * dq->type->t_size);
    if (new_container == NULL)
        return 0;
    dq->container = new_container;
    dq->size = mul_of_2_size;
    return 1;
}

/* Type functionality */
void *t_at_dequeue(const void *const src, const size_t index)
{
    return ((dequeue *)src) + index;
}

int t_cmp_dequeue(const void *const src_1, const void *const src_2)
{
    dequeue_t *a = data_shared_ptr(*(dequeue *)src_1);
    dequeue_t *b = data_shared_ptr(*(dequeue *)src_2);
    return a->size > b->size;
}

void *t_cpy_dequeue(void *const dest, const void *const src)
{
    if (*(dequeue *)dest != *(dequeue *)src)
        *(dequeue *)dest = copy_shared_ptr(*(dequeue *)src);
    return dest;
}

void *t_move_dequeue(void *const dest, const void *const src)
{
    *(dequeue *)dest = *(dequeue *)src;
    return dest;
}

void t_free_dequeue(void *const src)
{
    dequeue *val = (dequeue *)src;
    free_dequeue(*val);
    return;
}

size_t t_hash_dequeue(const void *const src)
{
    dequeue_t *dq = data_shared_ptr(*(dequeue *)src);
    size_t h = 0;
    h = hash_size_t(dq->count);
    h = hash_size_t(dq->size) ^ (h << 6) ^ (h >> 2);
    h = hash_size_t(dq->front) ^ (h << 6) ^ (h >> 2);
    h = hash_size_t(dq->back) ^ (h << 6) ^ (h >> 2);
    h = hash_size_t((size_t)dq->type) ^ (h << 6) ^ (h >> 2);
    h = hash_size_t((size_t)dq->container) ^ (h << 6) ^ (h >> 2);
    return h;
}

void t_swap_dequeue(void *const src_1, void *const src_2)
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

const type_func orig_f_dequeue = {
    .t_size = sizeof(dequeue),
    .t_at = t_at_dequeue,
    .t_cmp = t_cmp_dequeue,
    .t_cpy = t_cpy_dequeue,
    .t_move = t_move_dequeue,
    .t_free = t_free_dequeue,
    .t_hash = t_hash_dequeue,
    .t_swap = t_swap_dequeue};

const type_func *f_dequeue = &orig_f_dequeue;