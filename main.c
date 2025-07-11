#include "vector.h"
#include "rbt_map.h"
#include "hash_map.h"
#include "queue.h"
#include "dequeue.h"
#include "util_funcs.h"
#include "bit_types.h"
#include "string_type.h"
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
    ssize_t ad = 1422;
    b_ssize_t r = (b_ssize_t)ad;
    printf("%d\n", r.value);
    map_test();
    dequeue_test();
    queue_test();
    unordered_map_test();
    vector_test();
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
    vector *v = create_vector(0, f_uint16_t);
    uint16_t i = 1;
    for (; i < 16553; i++)
    {
        push_queue(q, &i);
        append_vector(v, &i);
    }

    uint16_t res_v;
    uint16_t res_q;
    i = 0;
    while (pop_queue(q, &res_q))
    {
        at_vector(v, i, &res_v);
        if (res_q !=  res_v)
            printf("Error queue\n");
        i++;
    }

    free_queue(q);
    free_vector(v);

    return;
}

void map_test(void)
{
    rbt_map *rbt = create_rbt_map(0, f_uint8_t, f_uint8_t);

    uint8_t key, val;

    key = 1; val = 10;
    set_rbt_map(rbt, &key, &val);

    key = 2; val = 20;
    set_rbt_map(rbt, &key, &val);

    key = 3; val = 30;
    set_rbt_map(rbt, &key, &val);

    key = 2; val = 40;
    set_rbt_map(rbt, &key, &val);

    key = 4; val = 20;
    set_rbt_map(rbt, &key, &val);

    key = 5; val = 50;
    set_rbt_map(rbt, &key, &val);

    key = 6; val = 60;
    set_rbt_map(rbt, &key, &val);

    key = 7; val = 70;
    set_rbt_map(rbt, &key, &val);

    key = 8; val = 80;
    set_rbt_map(rbt, &key, &val);

    key = 9; val = 90;
    set_rbt_map(rbt, &key, &val);

    key = 10; val = 100;
    set_rbt_map(rbt, &key, &val);

    key = 11; val = 110;
    set_rbt_map(rbt, &key, &val);

    key = 12; val = 120;
    set_rbt_map(rbt, &key, &val);

    key = 13; val = 130;
    set_rbt_map(rbt, &key, &val);

    key = 14; val = 140;
    set_rbt_map(rbt, &key, &val);

    key = 15; val = 150;
    set_rbt_map(rbt, &key, &val);

    key = 16; val = 160;
    set_rbt_map(rbt, &key, &val);

    key = 17; val = 170;
    set_rbt_map(rbt, &key, &val);

    key = 18; val = 180;
    set_rbt_map(rbt, &key, &val);

    key = 19; val = 190;
    set_rbt_map(rbt, &key, &val);

    key = 20; val = 200;
    set_rbt_map(rbt, &key, &val);

    free_rbt_map(rbt);

    return;
}

void unordered_map_test(void)
{
    uint8_t a = 13;

    unordered_map *b1 = create_hash_map(0, f_string, f_uint8_t);
    string ara = create_string("ara");
    string bruh = create_string("bruh");
    set_hash_map(b1, &ara, &a);

    if (has_key_hash_map(b1, &ara))
    {
        get_hash_map(b1, &ara, &a);
        printf("Have %s key with value %d\n", ara.data, a);
    }
    else
    {
        printf("No %s key\n", ara.data);
    }
    if (has_key_hash_map(b1, &bruh))
    {
        get_hash_map(b1, &bruh, &a);
        printf("Have %s key with value %d\n", bruh.data, a);
    }
    else
    {
        printf("No %s key\n", bruh.data);
    }
    delete_hash_map(b1, &ara, NULL);

    if (has_key_hash_map(b1, &ara))
    {
        get_hash_map(b1, &ara, &a);
        printf("Have %s key with value %d\n", ara.data, a);
    }
    else
    {
        printf("No %s key\n", ara.data);
    }
    a = 15;
    set_hash_map(b1, &ara, &a);

    if (has_key_hash_map(b1, &ara))
    {
        get_hash_map(b1, &ara, &a);
        printf("Have %s key with value %d\n", ara.data, a);
    }
    else
    {
        printf("No %s key\n", ara.data);
    }

    free_hash_map(b1);
    free_string(&ara);
    free_string(&bruh);

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

    at_vector(b, 2, &a);

    invert_vector(b);

    while (pop_vector(b, &a))
    {
        printf("%05.2f\n", a);
    }

    free_vector(b);

    return;
}
