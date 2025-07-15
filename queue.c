#include "queue.h"
#include "shared_ptr.h"
#include "util_funcs.h"
#include <stdlib.h>
#include <string.h>

#define QUEUE_MIN_SIZE 16

typedef struct queue
{
    size_t count;
    size_t size;
    size_t offset;
    const type_func* type;
    shared_ptr container;
} queue;

queue *create_queue(const size_t size, const type_func *const type)
{
    size_t mul_of_2_size = size < QUEUE_MIN_SIZE ? QUEUE_MIN_SIZE : next_power_of_2(size);
    queue new_q = {
        .count = 0,
        .size = mul_of_2_size,
        .offset = 0,
        .type = type,
        .container = calloc_shared_ptr(mul_of_2_size, type->t_size)};
    if (new_q.container == NULL)
        return NULL;
    return (queue *)memcpy(malloc(sizeof(queue)), &new_q, sizeof(queue));
}

size_t count_queue(const queue * const q)
{
    return q->count;
}

void free_queue(queue *const q)
{
    if (q->container != NULL)
    {
        ssize_t i = 0;
        void *container_data = data_shared_ptr(q->container);
        for (; i < q->count; i++)
        {
            q->type->t_free(q->type->t_at(container_data, i));
        }
        free_shared_ptr(q->container);
    }
    q->count = 0;
    q->size = 0;
    q->offset = 0;
    q->container = NULL;
    free(q);
    return;
}

static int expand_queue(queue *const q)
{
    if (++q->count <= q->size)
        return 1;
    size_t mul_of_2_size = next_power_of_2(q->count);
    shared_ptr new_container = realloc_shared_ptr(q->container, mul_of_2_size, q->type->t_size);
    if (new_container == NULL)
        return 0;
    void *container_data = data_shared_ptr(new_container);
    if (q->offset != 0)
    {
        size_t old_count = q->count - 1;
        size_t start_size = (old_count + q->offset) % q->size;
        memcpy(q->type->t_at(container_data, old_count), container_data, start_size * q->type->t_size);
        memcpy(container_data, q->type->t_at(container_data, q->offset), old_count * q->type->t_size);
        q->offset = 0;
    }
    memset(q->type->t_at(container_data, q->size), 0, (mul_of_2_size - q->size) * q->type->t_size);
    q->container = new_container;
    q->size = mul_of_2_size;
    return 1;
}

static int shrink_queue(queue *const q)
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
        void *container_data = data_shared_ptr(q->container);
        if ((q->count + q->offset) < q->size)
        {
            memcpy(container_data, q->type->t_at(container_data, q->offset), q->count * q->type->t_size);
        }
        else
        {
            size_t start_offset = (q->count + q->offset) % q->size;
            size_t end_size = q->count - start_offset;
            memcpy(q->type->t_at(container_data, start_offset), q->type->t_at(container_data, q->offset), end_size * q->type->t_size);
            memcpy(q->type->t_at(container_data, q->count), container_data, start_offset * q->type->t_size);
            memcpy(container_data, q->type->t_at(container_data, start_offset), q->count * q->type->t_size);
        }
        q->offset = 0;
    }
    shared_ptr new_container = realloc_shared_ptr(q->container, mul_of_2_size, q->type->t_size);
    if (new_container == NULL)
        return 0;
    q->container = new_container;
    q->size = mul_of_2_size;
    return 1;
}

int push_queue(queue *const q, const void* const val)
{
    if (val == NULL || q->count == SIZE_MAX || expand_queue(q) == 0)
        return 0;
    q->type->t_cpy(q->type->t_at(data_shared_ptr(q->container), (q->offset + q->count - 1) % q->size), val);
    return 1;
}

int pop_queue(queue *const q, void* const val)
{
    if (q->count == 0)
        return 0;
    void *container_val = q->type->t_at(data_shared_ptr(q->container), q->offset);
    if (val != NULL)
        q->type->t_cpy(val, container_val);
    q->type->t_free(container_val);
    q->offset = (q->offset + 1) % q->size;
    return shrink_queue(q);
}

int at_front_queue(queue *const q, void* const val)
{
    if (val == NULL || q->count == 0)
        return 0;
    q->type->t_cpy(val, q->type->t_at(data_shared_ptr(q->container), q->offset));
    return 1;
}
