#include "hash_funcs.h"

size_t djb2(const uint8_t *key, size_t key_len)
{
    size_t hash = 5381;
    for (; key_len--;)
    {
        hash = ((hash << 5) + hash) + (size_t)(*key++);
    }
    return hash;
}

size_t djb2_str(const uint8_t *key, size_t key_len)
{
    size_t hash = 5381;
    size_t c;
    while ((c = *key++))
    {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

size_t simple_hash(const uint8_t *key, size_t key_len)
{
    size_t i = 0;
    size_t offset = 0;
    size_t hash = 0;
    for (; i < key_len; i++, offset += 8)
    {
        hash |= (size_t)((*key++) << offset);
    }
    return hash;
}

size_t sdbm(const uint8_t *key, size_t key_len)
{
    size_t hash = 0;
    for (; key_len--;)
    {
        hash = (size_t)(*key++) + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}

size_t sdbm_str(const uint8_t *key, size_t key_len)
{
    size_t hash = 0;
    size_t c;
    while ((c = *key++))
    {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}

