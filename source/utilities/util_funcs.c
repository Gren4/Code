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

size_t calculate_padding(size_t currenaddres, size_t data_size)
{
    size_t offset = ((currenaddres + data_size - 1) / data_size) * data_size;
    return offset - currenaddres;
}

size_t hash_8bit(uint8_t x)
{
    x = ((x >> 3) ^ x) * 0x1D;
    x = ((x >> 5) ^ x) * 0x35;
    return x;
}

size_t hash_16bit(uint16_t x)
{
    x = ((x >> 8) ^ x) * 0x88CC;
    x = ((x >> 7) ^ x) * 0x119D;
    return x;
}

size_t hash_32bit(uint32_t x)
{
    x = ((x >> 16) ^ x) * 0x45D9F3B;
    x = ((x >> 16) ^ x) * 0x45D9F3B;
    return x ^ (x >> 16);
}

size_t hash_64bit(uint64_t x)
{
    x = ((x >> 30) ^ x) * 0xBF58476D1CE4E5B9;
    x = ((x >> 27) ^ x) * 0x94D049BB133111EB;
    return x ^ (x >> 31);
}

size_t hash_size_t(size_t x)
{
#if SIZE_MAX == UINT32_MAX
    x = ((x >> 16) ^ x) * 0x45D9F3B;
    x = ((x >> 16) ^ x) * 0x45D9F3B;
    return x ^ (x >> 16);
#else
    x = ((x >> 30) ^ x) * 0xBF58476D1CE4E5B9;
    x = ((x >> 27) ^ x) * 0x94D049BB133111EB;
    return x ^ (x >> 31);
#endif
}

size_t hash_string_t(const char *key)
{
    /* sdbm */
    size_t hash = 0;
    size_t c;
    while ((c = *key++))
    {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}