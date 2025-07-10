#include "deque.h"
#include "util_funcs.h"
#include <stdlib.h>
#include <string.h>

typedef struct dequeue
{
    size_t count;
    size_t size;
    size_t front;
    size_t back;
    const type_func* type;
    char *data;
} dequeue;

dequeue *create_dequeue(const size_t size, const type_func *const type)
{
    size_t mul_of_2_size = next_power_of_2(size);
    dequeue new_dq = {
        .count = 0,
        .size = mul_of_2_size,
        .front = mul_of_2_size >> 1,
        .back = mul_of_2_size >> 1,
        .type = type,
        .data = (char *)calloc(mul_of_2_size, type->t_size)};
    if (new_dq.data == NULL)
        return NULL;
    return (dequeue *)memcpy(malloc(sizeof(dequeue)), &new_dq, sizeof(dequeue));
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
    // if (dq->offset != 0)
    // {
    //     size_t start_size = (dq->count - 1 + dq->offset) % dq->size;
    //     char *temp_buff = (char*)malloc(start_size * dq->type->t_size);
    //     if (temp_buff == NULL)
    //         return 0;
    //     size_t end_size = dq->count - 1 - start_size;
    //     memcpy(temp_buff, new_data, start_size * dq->type->t_size);
    //     memcpy(new_data, dq->type->t_at(new_data, dq->offset), end_size * dq->type->t_size);
    //     memcpy(dq->type->t_at(new_data, end_size), temp_buff, start_size * dq->type->t_size);
    //     free(temp_buff);
    //     dq->offset = 0;
    // }
    memset(new_data + dq->size * dq->type->t_size, 0, (mul_of_2_size - dq->size) * dq->type->t_size);
    dq->data = new_data;
    dq->size = mul_of_2_size;
    return 1;
}

static int shrink_dequeue(dequeue *const dq)
{
    if (dq->count <= 0)
        return 0;
    if (--dq->count >= dq->size >> 3)
        return 1;
    // if (dq->offset != 0)
    // {
    //     size_t start_size = (dq->count + dq->offset) % dq->size;
    //     char *temp_buff = (char*)malloc(start_size * dq->type->t_size);
    //     if (temp_buff == NULL)
    //         return 0;
    //     size_t end_size = dq->count - start_size;
    //     memcpy(temp_buff, dq->data, start_size * dq->type->t_size);
    //     memcpy(dq->data, dq->type->t_at(dq->data, dq->offset), end_size * dq->type->t_size);
    //     memcpy(dq->type->t_at(dq->data, end_size), temp_buff, start_size * dq->type->t_size);
    //     free(temp_buff);
    //     dq->offset = 0;
    // }
    size_t mul_of_2_size = next_power_of_2(dq->count);
    char *new_data = realloc(dq->data, mul_of_2_size * dq->type->t_size);
    if (new_data == NULL)
        return 0;
    dq->data = new_data;
    dq->size = mul_of_2_size;
    return 1;
}

int push_front_dequeue(dequeue *const dq, const void* const val)
{
    if (expand_dequeue(dq) == 0)
        return 0;
    // dq->type->t_cpy(dq->type->t_at(dq->data, (dq->offset + dq->count - 1) % dq->size), val);
    return 1;
}

int push_back_dequeue(dequeue *const dq, const void* const val)
{
    return 1;
}

int pop_front_dequeue(dequeue *const dq, void* const val)
{
    return 1;
}

int pop_back_dequeue(dequeue *const dq, void* const val)
{
    if (dq->count <= 0)
        return 0;
    // char *ptr = dq->type->t_at(dq->data, dq->offset);
    if (val != NULL)
        dq->type->t_cpy(val, ptr);
    if (dq->type->t_free != NULL)
        dq->type->t_free(ptr);
    // dq->offset = (dq->offset + 1) % dq->size;
    return shrink_dequeue(dq);
}

