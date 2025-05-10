#include "unordered_map.h"
#include <stdlib.h>
#include <string.h>
#include "util_funcs.h"

typedef enum key_status
{
    EMPTY,
    USED,
    DELETED
} key_status;

unordered_map create_uo_map(const size_t key_size, const size_t data_size)
{
    unordered_map new_m = {
        .key_size = key_size,
        .data_size = data_size,
        .count = 0,
        .size = 4,
        .status = (void *)calloc(4, sizeof(key_status)),
        .keys = (void *)calloc(4, sizeof(size_t)),
        .data = (void *)calloc(4, data_size)};
    return new_m;
}

void free_uo_map(unordered_map *const m)
{
    if (m->status != nullptr)
        free(m->status);
    if (m->keys != nullptr)
        free(m->keys);
    if (m->data != nullptr)
        free(m->data);
    m->key_size = 0;
    m->data_size = 0;
    m->count = 0;
    m->size = 0;
    m->status = nullptr;
    m->keys = nullptr;
    m->data = nullptr;
    return;
}

static inline void *at_status(const unordered_map *const m, const size_t index)
{
    return m->status + index * sizeof(key_status);
}

static inline void *at_key(const unordered_map *const m, const size_t index)
{
    return m->keys + index * m->key_size;
}

static inline void *at_data(const unordered_map *const m, const size_t index)
{
    return m->data + index * m->data_size;
}

void resize_uo_map(unordered_map *const m, resize_type type)
{
    size_t i = 0;
    unordered_map new_m = {
        .key_size = m->key_size,
        .data_size = m->data_size,
        .count = m->count};
    if (type == EXPAND)
    {
        size_t new_size = next_power_of_2(m->size << 2);
        new_m.size = new_size;
        new_m.status = (void *)calloc(new_size, sizeof(key_status));
        new_m.keys = (void *)calloc(new_size, sizeof(size_t));
        new_m.data = (void *)calloc(new_size, m->data_size);
    }
    else
    {
        size_t new_size = next_power_of_2(m->size >> 2);
        if (new_size <= 4)
            return;
        new_m.size = new_size;
        new_m.status = (void *)calloc(new_size, sizeof(key_status));
        new_m.keys = (void *)calloc(new_size, sizeof(size_t));
        new_m.data = (void *)calloc(new_size, m->data_size);
    }
    for (; i < m->size; i++)
    {
        key_status *status = (key_status *)at_status(m, i);
        if (*status != USED)
            continue;
        void *key = at_key(m, i);
        void *data = at_data(m, i);
        size_t index = sdbm((uint8_t *)key, sizeof(size_t)) % new_m.size;
        size_t offset = 1;
        for (;;)
        {
            status = (key_status *)at_status(&new_m, index);
            if (*status != USED)
            {
                *status = USED;
                memcpy(at_key(&new_m, index), key, sizeof(size_t));
                memcpy(at_data(&new_m, index), data, new_m.data_size);
                break;
            }
            else
            {
                index = (index + offset++) % new_m.size;
            }
        }
    }
    free_uo_map(m);
    *m = new_m;
    return;
}

void set_uo_map(unordered_map *const m, const void *const key, const void *const val)
{
    if (m->count >= m->size)
        resize_uo_map(m, EXPAND);
    size_t hash_key = sdbm((uint8_t *)key, m->key_size);
    size_t index = sdbm((uint8_t *)&hash_key, sizeof(size_t)) % m->size;
    size_t i = index;
    size_t offset = 1;
    for (;;)
    {
        key_status *status = (key_status *)at_status(m, i);
        if (*status == USED && memcmp(at_key(m, i), &hash_key, sizeof(size_t)) == 0)
        {
            memcpy(at_data(m, i), val, m->data_size);
            return;
        }
        else
        {
            if (*status != USED)
            {
                *status = USED;
                memcpy(at_key(m, i), &hash_key, sizeof(size_t));
                memcpy(at_data(m, i), val, m->data_size);
                m->count++;
                return;
            }
            else
            {
                i = (i + offset++) % m->size;
                if (i == index)
                    return;
            }
        }
    }
    return;
}

void get_uo_map(unordered_map *const m, const void *const key, void *const val)
{
    size_t hash_key = sdbm((uint8_t *)key, m->key_size);
    size_t index = sdbm((uint8_t *)&hash_key, sizeof(size_t)) % m->size;
    size_t i = index;
    size_t offset = 1;
    for (;;)
    {
        key_status *status = (key_status *)at_status(m, i);
        if (*status == USED && memcmp(at_key(m, i), &hash_key, sizeof(size_t)) == 0)
        {
            memcpy(val, at_data(m, i), m->data_size);
            return;
        }
        else
        {
            i = (i + offset++) % m->size;
            if (i == index)
                return;
        }
    }
    return;
}

void delete_uo_map(unordered_map *const m, const void *const key, void *const val)
{
    size_t hash_key = sdbm((uint8_t *)key, m->key_size);
    size_t index = sdbm((uint8_t *)&hash_key, sizeof(size_t)) % m->size;
    size_t i = index;
    size_t offset = 1;
    for (;;)
    {
        key_status *status = (key_status *)at_status(m, i);
        if (*status == USED && memcmp(at_key(m, i), &hash_key, sizeof(size_t)) == 0)
        {
            if (val != nullptr)
                memcpy(val, at_data(m, i), m->data_size);
            *status = DELETED;
            m->count--;
            if (m->count < m->size >> 3)
                resize_uo_map(m, SHRINK);
            return;
        }
        else
        {
            i = (i + offset++) % m->size;
            if (i == index)
                return;
        }
    }
}

int has_key_uo_map(unordered_map *const m, const void *const key)
{
    size_t hash_key = sdbm((uint8_t *)key, m->key_size);
    size_t index = sdbm((uint8_t *)&hash_key, sizeof(size_t)) % m->size;
    size_t i = index;
    size_t offset = 1;
    for (;;)
    {
        key_status *status = (key_status *)at_status(m, i);
        if (*status == USED && memcmp(at_key(m, i), &hash_key, sizeof(size_t)) == 0)
        {
            return 1;
        }
        else if (*status == EMPTY)
        {
            return 0;
        }
        else
        {
            i = (i + offset++) % m->size;
            if (i == index)
                return 0;
        }
    }
}
