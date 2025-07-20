#include "queue.h"
#include "util_funcs.h"
#include <stdlib.h>
#include <string.h>

#define QUEUE_MIN_SIZE 16

/* Struct declaration */
typedef struct queue_t
{
    size_t count;
    size_t size;
    size_t offset;
    const type_func *type;
    void *container;
} queue_t;

/* Static function's declarations */
static int expand_queue(queue_t *const q);
static int shrink_queue(queue_t *const q);

queue create_queue(const size_t size, const type_func *const type)
{
    queue ptr = malloc_shared_ptr(1, sizeof(queue_t));
    if (ptr == NULL)
        return NULL;
    size_t mul_of_2_size = size < QUEUE_MIN_SIZE ? QUEUE_MIN_SIZE : next_power_of_2(size);
    queue_t new_q = {
        .count = 0,
        .size = mul_of_2_size,
        .offset = 0,
        .type = type,
        .container = calloc(mul_of_2_size, type->t_size)};
    if (new_q.container == NULL)
    {
        free_shared_ptr(ptr);
        return NULL;
    }
    memcpy(data_shared_ptr(ptr), &new_q, sizeof(queue_t));
    return ptr;
}

size_t count_queue(const queue ptr)
{
    if (ptr == NULL)
        return 0;
    queue_t *q = data_shared_ptr(ptr);
    return q->count;
}

void free_queue(queue const ptr)
{
    if (ptr == NULL)
        return;
    if (count_shared_ptr(ptr) == 1)
    {
        queue_t *q = data_shared_ptr(ptr);
        if (q->container != NULL)
        {
            size_t i = 0;
            for (; i < q->count; i++)
            {
                q->type->t_free(q->type->t_at(q->container, i));
            }
            free(q->container);
        }
        q->count = 0;
        q->size = 0;
        q->offset = 0;
        q->container = NULL;
    }
    free_shared_ptr(ptr);
    return;
}

int push_queue(queue const ptr, const void *const val)
{
    if (ptr == NULL)
        return 0;
    queue_t *q = data_shared_ptr(ptr);
    if (val == NULL || q->count == SIZE_MAX || expand_queue(q) == 0)
        return 0;
    q->type->t_cpy(q->type->t_at(q->container, (q->offset + q->count - 1) % q->size), val);
    return 1;
}

int pop_queue(queue const ptr, void *const val)
{
    if (ptr == NULL)
        return 0;
    queue_t *q = data_shared_ptr(ptr);
    if (q->count == 0)
        return 0;
    void *container_val = q->type->t_at(q->container, q->offset);
    if (val != NULL)
        q->type->t_cpy(val, container_val);
    q->type->t_free(container_val);
    q->offset = (q->offset + 1) % q->size;
    return shrink_queue(q);
}

int at_front_queue(queue const ptr, void *const val)
{
    if (ptr == NULL)
        return 0;
    queue_t *q = data_shared_ptr(ptr);
    if (val == NULL || q->count == 0)
        return 0;
    q->type->t_cpy(val, q->type->t_at(q->container, q->offset));
    return 1;
}

/* Static functions */
static int expand_queue(queue_t *const q)
{
    if (++q->count <= q->size)
        return 1;
    size_t mul_of_2_size = next_power_of_2(q->count);
    void *new_container = realloc(q->container, mul_of_2_size * q->type->t_size);
    if (new_container == NULL)
        return 0;
    if (q->offset != 0)
    {
        size_t old_count = q->count - 1;
        size_t start_size = (old_count + q->offset) % q->size;
        memcpy(q->type->t_at(new_container, old_count), new_container, start_size * q->type->t_size);
        memcpy(new_container, q->type->t_at(new_container, q->offset), old_count * q->type->t_size);
        q->offset = 0;
    }
    memset(q->type->t_at(new_container, q->size), 0, (mul_of_2_size - q->size) * q->type->t_size);
    q->container = new_container;
    q->size = mul_of_2_size;
    return 1;
}

static int shrink_queue(queue_t *const q)
{
    if (--q->count > q->size >> 3)
        return 1;
    if (q->count == 0)
        q->offset = 0;
    size_t mul_of_2_size = next_power_of_2(q->count);
    if (mul_of_2_size <= QUEUE_MIN_SIZE)
        return 1;
    if (q->offset != 0)
    {
        if ((q->count + q->offset) < q->size)
        {
            memcpy(q->container, q->type->t_at(q->container, q->offset), q->count * q->type->t_size);
        }
        else
        {
            size_t start_offset = (q->count + q->offset) % q->size;
            size_t end_size = q->count - start_offset;
            memcpy(q->type->t_at(q->container, start_offset), q->type->t_at(q->container, q->offset), end_size * q->type->t_size);
            memcpy(q->type->t_at(q->container, q->count), q->container, start_offset * q->type->t_size);
            memcpy(q->container, q->type->t_at(q->container, start_offset), q->count * q->type->t_size);
        }
        q->offset = 0;
    }
    void *new_container = realloc(q->container, mul_of_2_size * q->type->t_size);
    if (new_container == NULL)
        return 0;
    q->container = new_container;
    q->size = mul_of_2_size;
    return 1;
}

/* Type functionality */
void *t_at_queue(const void *const src, const size_t index)
{
    return ((queue *)src) + index;
}

int t_cmp_queue(const void *const src_1, const void *const src_2)
{
    queue_t *a = data_shared_ptr(*(queue *)src_1);
    queue_t *b = data_shared_ptr(*(queue *)src_2);
    return a->size > b->size;
}

void *t_cpy_queue(void *const dest, const void *const src)
{
    if (*(queue *)dest != *(queue *)src)
        *(queue *)dest = copy_shared_ptr(*(queue *)src);
    return dest;
}

void *t_move_queue(void *const dest, const void *const src)
{
    *(queue *)dest = *(queue *)src;
    return dest;
}

void t_free_queue(void *const src)
{
    queue *val = (queue *)src;
    free_queue(*val);
    return;
}

size_t t_hash_queue(const void *const src)
{
    queue_t *q = data_shared_ptr(*(queue *)src);
    size_t h = 0;
    h = hash_size_t(q->count);
    h = hash_size_t(q->size) ^ (h << 6) ^ (h >> 2);
    h = hash_size_t(q->offset) ^ (h << 6) ^ (h >> 2);
    h = hash_size_t((size_t)q->type) ^ (h << 6) ^ (h >> 2);
    h = hash_size_t((size_t)q->container) ^ (h << 6) ^ (h >> 2);
    return h;
}

void t_swap_queue(void *const src_1, void *const src_2)
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

const type_func orig_f_queue = {
    .t_size = sizeof(queue),
    .t_at = t_at_queue,
    .t_cmp = t_cmp_queue,
    .t_cpy = t_cpy_queue,
    .t_move = t_move_queue,
    .t_free = t_free_queue,
    .t_hash = t_hash_queue,
    .t_swap = t_swap_queue};

const type_func *f_queue = &orig_f_queue;
