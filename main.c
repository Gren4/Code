#include "vector.h"
#include "rbt_map.h"
#include "hash_map.h"
#include "queue.h"
#include "dequeue.h"
#include "util_funcs.h"
#include "bit_types.h"
#include "string_type.h"
#include "shared_ptr.h"
#include <stdio.h>
#include <stdlib.h>

#define N 50

void vector_test(void);
void map_test(void);
void unordered_map_test(void);
void queue_test(void);
void dequeue_test(void);
int main(void)
{
    map_test();
//    dequeue_test();
//    queue_test();
//    unordered_map_test();
//    vector_test();
    printf("Done\n");
    return 1;
}

void dequeue_test(void)
{
    dequeue *dq = create_dequeue(2, f_uint8_t);

    uint8_t a = 115;

    for (; a > 0; a--)
    {
        push_front_dequeue(dq, &a);
    }

    for (a = 116; a <= 230; a++)
    {
        push_back_dequeue(dq, &a);
    }

    uint8_t b = 0;
    while(pop_back_dequeue(dq, &b))
    {
        printf("%d\n", b);
        if (pop_front_dequeue(dq, &b))
            printf("%d\n", b);
    }

    free_dequeue(dq);

    return;
}

void queue_test(void)
{
    queue *q = create_queue(0, f_uint16_t);
    uint16_t i = 1;
    for (; i < 256; i++)
    {
        push_queue(q, &i);
    }
    for (; i > 15; i--)
    {
        pop_queue(q, 0);
    }
    for (; i < 256; i++)
    {
        push_queue(q, &i);
    }
    while (pop_queue(q, &i))
    {
        printf("Queue %d\n", i);
    }

    free_queue(q);

    return;
}
#define RBT 512
void map_test(void)
{
    rbt_map *rbt = create_rbt_map(0, f_uint16_t, f_uint16_t);

    uint16_t key = 0, val = RBT;

    for (; key < RBT; key++, val--)
    {
        set_rbt_map(rbt, &key, &val);
    }

    for (key = 0; key < 385; key++)
    {
        delete_rbt_map(rbt, &key, 0);
    }

//    for (key = 0; key < RBT; key++)
//    {
//        if (get_rbt_map(rbt, &key, &val) == 1)
//            printf("RBT Map has key %d with a value %d\n", key, val);
//        else
//            printf("RBT Map doesn't have key %d\n", key);
//    }

    free_rbt_map(rbt);

    return;
}

void unordered_map_test(void)
{
    unordered_map *hm = create_hash_map(0, f_uint8_t, f_uint8_t);
    uint8_t a = 0;
    uint8_t b = 255;
    for (a = 0; a < 255; a++, b--)
    {
        set_hash_map(hm, &a, &b);
    }

    for (a = 1; a < 255; a+=2, b--)
    {
        delete_hash_map(hm, &a, &b);
    }

    for (a = 0; a < 255; a++)
    {
        if (get_hash_map(hm, &a, &b) == 0)
            printf("Hash map doesn't have a key %d\n", a);
        else
            printf("Hash map has a key %d\n", a);
    }

    free_hash_map(hm);
    return;
}

void vector_test(void)
{
    size_t i = 0;
    double a = 0.0;
    vector *b = create_vector(0, f_double);

    for (i = 0; i < N; i++)
    {
        a += 1.0;
        append_vector(b, (void *)&a);
    }

    a = 99.0;
    insert_vector(b, 5, (void *)&a);
    delete_vector(b, 5, NULL);

    get_vector(b, 2, &a);

    invert_vector(b);

    while (pop_vector(b, &a))
    {
        printf("%05.2f\n", a);
    }

    free_vector(b);

    return;
}
