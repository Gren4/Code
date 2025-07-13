#include "util_funcs.h"

size_t next_power_of_2(size_t num)
{
    if (num <= 1)
    {
        return 2;
    }
    else if (num % 2 == 0)
    {
        return num;
    }
    else
    {
        --num;
        num |= num >> 1;
        num |= num >> 2;
        num |= num >> 4;
        num |= num >> 8;
        num |= num >> 16;
        num |= num >> 32;
        return ++num;
    }
}

size_t calculate_padding(size_t current_addres, size_t data_size)
{
    size_t offset = ((current_addres + data_size - 1) / data_size) * data_size;
    return offset - current_addres;
}
