#include "dequeue.h"
#include "util_funcs.h"
#include <stdlib.h>
#include <string.h>

typedef struct dequeue
{
    size_t count;
    size_t size;
    size_t front;
    size_t back;
    const type_func *type;
    char *data;
} dequeue;

dequeue *create_dequeue(const size_t size, const type_func *const type)
{
    size_t mul_of_2_size = next_power_of_2(size);
    dequeue new_dq = {
        .count = 0,
        .size = mul_of_2_size,
        .front = 0,
        .back = 0,
        .type = type,
        .data = (char *)calloc(mul_of_2_size, type->t_size)};
    if (new_dq.data == NULL)
        return NULL;
    return (dequeue *)memcpy(malloc(sizeof(dequeue)), &new_dq, sizeof(dequeue));
}

size_t count_dequeue(const dequeue * const dq)
{
    return dq->count;
}

void free_dequeue(dequeue *const dq)
{
    if (dq->data != NULL)
    {
        if (dq->type->t_free != NULL && dq->count > 0)
        {
            ssize_t i = 0;
            for (; i < dq->count; i++)
            {
                dq->type->t_free(dq->type->t_at(dq->data, i));
            }
        }
        free(dq->data);
    }
    dq->count = 0;
    dq->size = 0;
    dq->front = 0;
    dq->back = 0;
    dq->data = NULL;
    return;
}

static int expand_dequeue(dequeue *const dq)
{
    if (++dq->count <= dq->size)
        return 1;
    size_t mul_of_2_size = next_power_of_2(dq->count);
    char *new_data = realloc(dq->data, mul_of_2_size * dq->type->t_size);
    if (new_data == NULL)
        return 0;
    if (dq->front != 0)
    {
        size_t old_count = dq->count - 1;
        size_t start_offset = dq->back + 1;
        memcpy(dq->type->t_at(new_data, old_count), new_data, start_offset * dq->type->t_size);
        memcpy(new_data, dq->type->t_at(new_data, start_offset), old_count * dq->type->t_size);
        dq->front = 0;
        dq->back = old_count - 1;
    }
    memset(new_data + dq->size * dq->type->t_size, 0, (mul_of_2_size - dq->size) * dq->type->t_size);
    dq->data = new_data;
    dq->size = mul_of_2_size;
    return 1;
}

static int shrink_dequeue(dequeue *const dq)
{
    if (--dq->count > dq->size >> 3)
        return 1;
    if (dq->count == 0)
    {
        dq->front = 0;
        dq->back = 0;
    }
    else if (dq->front != 0)
    {
        if (dq->back > dq->front)
        {
            memcpy(dq->data, dq->type->t_at(dq->data, dq->front), dq->count * dq->type->t_size);
        }
        else
        {
            size_t start_offset = dq->back + 1;
            size_t end_size = dq->count - start_offset;
            memcpy(dq->type->t_at(dq->data, start_offset), dq->type->t_at(dq->data, dq->front), end_size * dq->type->t_size);
            memcpy(dq->type->t_at(dq->data, dq->count), dq->data, start_offset * dq->type->t_size);
            memcpy(dq->data, dq->type->t_at(dq->data, start_offset), dq->count * dq->type->t_size);
        }
        dq->front = 0;
        dq->back = dq->count - 1;
    }
    size_t mul_of_2_size = next_power_of_2(dq->count);
    char *new_data = realloc(dq->data, mul_of_2_size * dq->type->t_size);
    if (new_data == NULL)
        return 0;
    dq->data = new_data;
    dq->size = mul_of_2_size;
    return 1;
}

int push_front_dequeue(dequeue *const dq, const void *const val)
{
    if (val == NULL || dq->count == SIZE_MAX || expand_dequeue(dq) == 0)
        return 0;
    if (dq->front != dq->back || dq->count > 1)
        dq->front = dq->front == 0 ? dq->size - 1 : dq->front - 1;
    dq->type->t_cpy(dq->type->t_at(dq->data, dq->front), val);
    return 1;
}

int push_back_dequeue(dequeue *const dq, const void *const val)
{
    if (val == NULL || dq->count == SIZE_MAX || expand_dequeue(dq) == 0)
        return 0;
    if (dq->back != dq->front || dq->count > 1)
        dq->back = (dq->back + 1) % dq->size;
    dq->type->t_cpy(dq->type->t_at(dq->data, dq->back), val);
    return 1;
}

int pop_front_dequeue(dequeue *const dq, void *const val)
{
    if (dq->count == 0)
        return 0;
    char *ptr = dq->type->t_at(dq->data, dq->front);
    if (val != NULL)
        dq->type->t_cpy(val, ptr);
    if (dq->type->t_free != NULL)
        dq->type->t_free(ptr);
    dq->front = (dq->front + 1) % dq->size;
    return shrink_dequeue(dq);
}

int pop_back_dequeue(dequeue *const dq, void *const val)
{
    if (dq->count == 0)
        return 0;
    char *ptr = dq->type->t_at(dq->data, dq->back);
    if (val != NULL)
        dq->type->t_cpy(val, ptr);
    if (dq->type->t_free != NULL)
        dq->type->t_free(ptr);
    dq->back = dq->back == 0 ? dq->size - 1 : dq->back - 1;
    return shrink_dequeue(dq);
}

int at_front_dequeue(dequeue *const dq, void *const val)
{
    if (dq->count == 0)
        return 0;
    char *ptr = dq->type->t_at(dq->data, dq->front);
    if (val != NULL)
        dq->type->t_cpy(val, ptr);
    return 1;
}

int at_back_dequeue(dequeue *const dq, void *const val)
{
    if (dq->count == 0)
        return 0;
    char *ptr = dq->type->t_at(dq->data, dq->back);
    if (val != NULL)
        dq->type->t_cpy(val, ptr);
    return 1;
}