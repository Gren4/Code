#include "vector.h"
#include "unordered_map.h"
#include "util_funcs.h"
#include <stdio.h>

#define N 5000000

void vector_test(void);
void unordered_map_test(void);

int main(void)
{
    unordered_map_test();
    vector_test();

    return 1;
}

void unordered_map_test(void)
{
    uint8_t a = 13;

    unordered_map b1 = create_uo_map(string_key_size, sizeof(uint8_t), 0);

    set_uo_map(&b1, "ara", &a);

    if (has_key_uo_map(&b1, "ara"))
    {
        get_uo_map(&b1, "ara", &a);
        printf("Have %s key with value %d\n", "ara", a);
    }
    else
    {
        printf("No %s key\n", "ara");
    }
    if (has_key_uo_map(&b1, "bruh"))
    {
        get_uo_map(&b1, "bruh", &a);
        printf("Have %s key with value %d\n", "bruh", a);
    }
    else
    {
        printf("No %s key\n", "bruh");
    }
    delete_uo_map(&b1, "ara", nullptr);

    if (has_key_uo_map(&b1, "ara"))
    {
        get_uo_map(&b1, "ara", &a);
        printf("Have %s key with value %d\n", "ara", a);
    }
    else
    {
        printf("No %s key\n", "ara");
    }

    free_uo_map(&b1);

    unordered_map b2 = create_uo_map(sizeof(size_t), sizeof(uint8_t), N);

    size_t i = 0;
    a = 2;
    for (i = 0; i < N; i++, a++)
    {
        set_uo_map(&b2, &i, &a);
    }
    printf("Added %d elements to uo map\n", N);
    i = N >> 1;
    if (has_key_uo_map(&b2, &i))
    {
        get_uo_map(&b2, &i, &a);
        printf("Have %llu key with value %d\n", i, a);
    }
    else
    {
        printf("No %llu key\n", i);
    }
    free_uo_map(&b2);

    return;
}

void vector_test(void)
{
    size_t i = 0;
    double a = 0.0;
    vector b = create_vector(0, sizeof(double));

    for (i = 0; i < N; i++)
    {
        a += 1.0;
        append_vector(&b, (void *)&a);
    }

    a = 99.0;
    insert_vector(&b, 5, (void *)&a);
    delete_vector(&b, 5, nullptr);

    a = *((double *)at_vector(&b, 2));

    invert_vector(&b);

    a = 2.0;
    printf("2.0 is at index %llu\n", find_vector(&b, &a));

    while(pop_vector(&b, &a))
    {
//        printf("%05.2f\n", a);
    }

    free_vector(&b);

    return;
}
