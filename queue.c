#include "queue.h"
#include "util_funcs.h"
#include <stdlib.h>
#include <string.h>

typedef struct queue
{
    size_t count;
    size_t size;
    size_t offset;
    const type_func* type;
    char *data;
} queue;

queue *create_queue(const size_t size, const type_func *const type)
{
    size_t mul_of_2_size = next_power_of_2(size);
    queue new_q = {
        .count = 0,
        .size = mul_of_2_size,
        .offset = 0,
        .type = type,
        .data = (char *)calloc(mul_of_2_size, type->t_size)};
    if (new_q.data == NULL)
        return NULL;
    return (queue *)memcpy(malloc(sizeof(queue)), &new_q, sizeof(queue));
}

size_t count_queue(const queue * const q)
{
    return q->count;
}

void free_queue(queue *const q)
{
    if (q->data != NULL)
    {
        if (q->count > 0)
        {
            ssize_t i = 0;
            for (; i < q->count; i++)
            {
                q->type->t_free(q->type->t_at(q->data, i));
            }
        }
        free(q->data);
    }
    q->count = 0;
    q->size = 0;
    q->offset = 0;
    q->data = NULL;
    return;
}

static int expand_queue(queue *const q)
{
    if (++q->count <= q->size)
        return 1;
    size_t mul_of_2_size = next_power_of_2(q->count);
    char *new_data = realloc(q->data, mul_of_2_size * q->type->t_size);
    if (new_data == NULL)
        return 0;
    if (q->offset != 0)
    {
        size_t old_count = q->count - 1;
        size_t start_size = (old_count + q->offset) % q->size;
        memcpy(q->type->t_at(new_data, old_count), new_data, start_size * q->type->t_size);
        memcpy(new_data, q->type->t_at(new_data, q->offset), old_count * q->type->t_size);
        q->offset = 0;
    }
    memset(new_data + q->size * q->type->t_size, 0, (mul_of_2_size - q->size) * q->type->t_size);
    q->data = new_data;
    q->size = mul_of_2_size;
    return 1;
}

static int shrink_queue(queue *const q)
{
    if (--q->count > q->size >> 3)
        return 1;
    if (q->count == 0)
    {
        q->offset = 0;
    }
    else if (q->offset != 0)
    {
        if ((q->count + q->offset) < q->size)
        {
            memcpy(q->data, q->type->t_at(q->data, q->offset), q->count * q->type->t_size);
        }
        else
        {
            size_t start_offset = (q->count + q->offset) % q->size;
            size_t end_size = q->count - start_offset;
            memcpy(q->type->t_at(q->data, start_offset), q->type->t_at(q->data, q->offset), end_size * q->type->t_size);
            memcpy(q->type->t_at(q->data, q->count), q->data, start_offset * q->type->t_size);
            memcpy(q->data, q->type->t_at(q->data, start_offset), q->count * q->type->t_size);
        }
        q->offset = 0;
    }
    size_t mul_of_2_size = next_power_of_2(q->count);
    char *new_data = realloc(q->data, mul_of_2_size * q->type->t_size);
    if (new_data == NULL)
        return 0;
    q->data = new_data;
    q->size = mul_of_2_size;
    return 1;
}

int push_queue(queue *const q, const void* const val)
{
    if (val == NULL || q->count == SIZE_MAX || expand_queue(q) == 0)
        return 0;
    q->type->t_cpy(q->type->t_at(q->data, (q->offset + q->count - 1) % q->size), val);
    return 1;
}

int pop_queue(queue *const q, void* const val)
{
    if (q->count == 0)
        return 0;
    char *ptr = q->type->t_at(q->data, q->offset);
    if (val != NULL)
        q->type->t_cpy(val, ptr);
    q->type->t_free(ptr);
    q->offset = (q->offset + 1) % q->size;
    return shrink_queue(q);
}

int at_front_queue(queue *const q, void* const val)
{
    if (val == NULL || q->count == 0)
        return 0;
    char *ptr = q->type->t_at(q->data, q->offset);
    q->type->t_cpy(val, ptr);
    return 1;
}
