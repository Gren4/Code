#include "vector.h"
#include "rbt_map.h"
#include "hash_map.h"
#include "queue.h"
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

int main(void)
{
//    float ad = 1422;
//    bfloat r = (bfloat)ad;
//    printf("%f\n", r.value);
//    map_test();
//    queue_test();
//    unordered_map_test();
//    vector_test();

    return 1;
}

void queue_test(void)
{
    queue *q = create_queue(4, f_uint8_t);
    uint8_t a = 1;
    uint8_t b = 0;
    ssize_t i = 0;
    for (i = 0; i < 8; i++, a++)
    {
        push_queue(q, &a);
    }
    for (i = 0; i < 3; i++)
    {
        pop_queue(q, &b);
    }
    for (i = 0; i < 3; i++, a++)
    {
        push_queue(q, &a);
    }

    push_queue(q, &a);

    free_queue(q);

    return;
}

void map_test(void)
{
//    rbt_map *b = create_map(sizeof(uint8_t), sizeof(uint8_t));
//    uint8_t key = 3, val = 1;
//    set_map(b, &key, &val);
//    key = 3;
//    val = 2;
//    set_map(b, &key, &val);
//    key = 2;
//    val = 3;
//    set_map(b, &key, &val);
//    key = 1;
//    val = 4;
//    set_map(b, &key, &val);
//    key = 5;
//    val = 5;
//    set_map(b, &key, &val);
//    key = 6;
//    val = 6;
//    set_map(b, &key, &val);
//    key = 5;
//    delete_map(b, &key, &val);
//    free_map(b);

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

//    unordered_map *b2 = create_hash_map(N, f_size_t, f_uint8_t);
//
//    size_t i = 0;
//    a = 2;
//    for (i = 0; i < N; i++, a++)
//    {
//        set_hash_map(b2, &i, &a);
//    }
//    printf("Added %d elements to hash map\n", N);
//    i = N >> 1;
//    if (has_key_hash_map(b2, &i))
//    {
//        get_hash_map(b2, &i, &a);
//        printf("Have %llu key with value %d\n", i, a);
//    }
//    else
//    {
//        printf("No %llu key\n", i);
//    }
//
//    for (i = N - 1; i > N >> 1; i--)
//    {
//        delete_hash_map(b2, &i, 0);
//    }
//    i = N >> 1;
//    if (has_key_hash_map(b2, &i))
//    {
//        get_hash_map(b2, &i, &a);
//        printf("Have %llu key with value %d\n", i, a);
//    }
//    else
//    {
//        printf("No %llu key\n", i);
//    }
//    i = (N >> 1) + 1;
//    if (has_key_hash_map(b2, &i))
//    {
//        get_hash_map(b2, &i, &a);
//        printf("Have %llu key with value %d\n", i, a);
//    }
//    else
//    {
//        printf("No %llu key\n", i);
//    }
//
//    free_hash_map(b2);

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

    a = *((double *)at_vector(b, 2));

    invert_vector(b);

    while (pop_vector(b, &a))
    {
        printf("%05.2f\n", a);
    }

    free_vector(b);

    return;
}
