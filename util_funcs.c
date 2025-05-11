#include "util_funcs.h"

size_t next_power_of_2(size_t num)
{
    if (num <= 1)
    {
        return 2;
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

size_t djb2(uint8_t *key, size_t key_len)
{
    if (key_len > 0)
    {
        size_t hash = 5381;
        for (; key_len--;)
        {
            hash = ((hash << 5) + hash) + (size_t)(*key++);
        }
        return hash;
    }
    else
    {
        size_t hash = 5381;
        size_t c;
        while ((c = *key++))
        {
            hash = ((hash << 5) + hash) + c;
        }
        return hash;
    }
}

size_t sdbm(uint8_t *key, size_t key_len)
{
    if (key_len > 0)
    {
        size_t hash = 0;
        for (; key_len--;)
        {
            hash = (size_t)(*key++) + (hash << 6) + (hash << 16) - hash;
        }
        return hash;
    }
    else
    {
        size_t hash = 0;
        size_t c;
        while ((c = *key++))
        {
            hash = c + (hash << 6) + (hash << 16) - hash;
        }
        return hash;
    }
}
