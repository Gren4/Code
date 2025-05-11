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

static inline void *at_status(const unordered_map *const m, const size_t index)
{
    return m->status + index * sizeof(key_status);
}

static inline void *at_key(const unordered_map *const m, const size_t index)
{
    if (m->key_size == string_key_size)
        return m->keys + index * sizeof(char *);
    else
        return m->keys + index * m->key_size;
}

static inline void *at_data(const unordered_map *const m, const size_t index)
{
    return m->data + index * m->data_size;
}

static inline void *at_status_p(void *const status, const size_t index)
{
    return status + index * sizeof(key_status);
}

static inline void *at_key_p(void *const keys, const size_t index, const size_t key_size)
{
    if (key_size == string_key_size)
        return keys + index * sizeof(char *);
    else
        return keys + index * key_size;
}

static inline void *at_data_p(void *const data, const size_t index, const size_t data_size)
{
    return data + index * data_size;
}

static inline int key_cmp(const void *const a, const void *const b, const size_t size)
{
    if (size == string_key_size)
        return strcmp(a, b);
    else
        return memcmp(a, b, size);
}

static inline void key_cpy(void *const a, const void *const b, const size_t size)
{
    if (size == string_key_size)
    {

        char *string = strcpy(calloc(strlen(b) + 1, sizeof(char)), b);
        memcpy(a, &string, sizeof(char *));
    }
    else
        memcpy(a, b, size);
    return;
}

unordered_map create_uo_map(const size_t key_size, const size_t data_size, const size_t size)
{
    size_t new_size = size < 4 ? 4 : next_power_of_2(size);
    unordered_map new_m = {
        .key_size = key_size,
        .data_size = data_size,
        .count = 0,
        .size = new_size,
        .status = (void *)calloc(new_size, sizeof(key_status)),
        .keys = key_size != string_key_size ? (void *)calloc(new_size, key_size) : (void *)calloc(new_size, sizeof(char *)),
        .data = (void *)calloc(new_size, data_size),
        .hash_function = key_size != string_key_size ? (key_size <= sizeof(size_t) ? simple_hash : sdbm) : sdbm_str};
    return new_m;
}

void free_uo_map(unordered_map *const m)
{
    if (m->status != nullptr)
        free(m->status);
    if (m->keys != nullptr)
    {
        if (m->key_size == string_key_size)
        {
            size_t i = 0;
            for (; i < m->size; i++)
            {
                if (*(key_status *)at_status(m, i) == USED)
                {
                    free(*(char **)at_key(m, i));
                }
            }
        }
        free(m->keys);
    }
    if (m->data != nullptr)
        free(m->data);
    m->count = 0;
    m->size = 0;
    m->status = nullptr;
    m->keys = nullptr;
    m->data = nullptr;
    return;
}

static void resize_uo_map(unordered_map *const m, resize_type type)
{
    size_t i = 0;
    size_t new_size;
    void *new_status;
    void *new_keys;
    void *new_data;
    if (type == EXPAND)
    {
        new_size = next_power_of_2(m->size << 1);
    }
    else
    {
        new_size = next_power_of_2(m->size >> 1);
        if (new_size <= 4)
            return;
    }
    new_status = (void *)calloc(new_size, sizeof(key_status));
    new_keys = m->key_size != string_key_size ? (void *)calloc(new_size, m->key_size) : (void *)calloc(new_size, sizeof(char *));
    new_data = (void *)calloc(new_size, m->data_size);
    for (; i < m->size; i++)
    {
        if (*(key_status *)at_status(m, i) != USED)
            continue;
        void *key = at_key(m, i);
        void *data = at_data(m, i);
        size_t index = m->hash_function((uint8_t *)key, m->key_size) % new_size;
        size_t offset = 1;
        for (;;)
        {
            key_status *status = (key_status *)at_status_p(new_status, index);
            if (*status != USED)
            {
                *status = USED;
                memcpy(at_key(new_keys, index), key, m->key_size);
                memcpy(at_data_p(new_data, index, m->data_size), data, m->data_size);
                break;
            }
            else
            {
                index = (index + offset++) % new_size;
            }
        }
    }
    free(m->status);
    free(m->keys);
    free(m->data);
    m->status = new_status;
    m->keys = new_keys;
    m->data = new_data;
    m->size = new_size;
    return;
}

void set_uo_map(unordered_map *const m, const void *const key, const void *const val)
{
    if (m->count >= m->size)
        resize_uo_map(m, EXPAND);
    size_t index = m->hash_function((uint8_t *)key, m->key_size) % m->size;
    size_t i = index;
    size_t offset = 1;
    for (;;)
    {
        key_status *status = (key_status *)at_status(m, i);
        if (*status == USED && key_cmp(at_key(m, i), key, m->key_size) == 0)
        {
            memcpy(at_data(m, i), val, m->data_size);
            return;
        }
        else
        {
            if (*status != USED)
            {
                *status = USED;
                key_cpy(at_key(m, i), key, m->key_size);
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

void get_uo_map(const unordered_map *const m, const void *const key, void *const val)
{
    size_t index = m->hash_function((uint8_t *)key, m->key_size) % m->size;
    size_t i = index;
    size_t offset = 1;
    for (;;)
    {
        key_status *status = (key_status *)at_status(m, i);
        if (*status == USED && key_cmp(at_key(m, i), key, m->key_size) == 0)
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
    size_t index = m->hash_function((uint8_t *)key, m->key_size) % m->size;
    size_t i = index;
    size_t offset = 1;
    for (;;)
    {
        key_status *status = (key_status *)at_status(m, i);
        if (*status == USED && key_cmp(at_key(m, i), key, m->key_size) == 0)
        {
            if (val != nullptr)
                memcpy(val, at_data(m, i), m->data_size);
            *status = DELETED;
            m->count--;
            if (m->key_size == string_key_size)
                free(*(char **)at_key(m, i));
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

int has_key_uo_map(const unordered_map *const m, const void *const key)
{
    size_t index = m->hash_function((uint8_t *)key, m->key_size) % m->size;
    size_t i = index;
    size_t offset = 1;
    for (;;)
    {
        key_status *status = (key_status *)at_status(m, i);
        if (*status == USED && key_cmp(at_key(m, i), key, m->key_size) == 0)
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
