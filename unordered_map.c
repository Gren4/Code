#include "unordered_map.h"
#include <stdlib.h>
#include <string.h>
#include "util_funcs.h"
#include "hash_funcs.h"

typedef enum key_status
{
    EMPTY,
    USED,
    DELETED
} key_status;

typedef struct unordered_map
{
    const size_t key_size;
    const size_t data_size;
    size_t count;
    size_t size;
    key_status *status;
    void *keys;
    void *data;
    size_t (*hash_function)(const uint8_t *key, size_t key_len);
} unordered_map;

static inline key_status *at_status(const unordered_map *const m, const size_t index)
{
    return m->status + index;
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

static inline key_status *at_status_p(key_status *const status, const size_t index)
{
    return status + index;
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
        return strcmp(*(char **)a, b);
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

unordered_map *create_uo_map(const size_t key_size, const size_t data_size, const size_t size)
{
    size_t new_size = size < 4 ? 4 : next_power_of_2(size);
    unordered_map new_m = {
        .key_size = key_size,
        .data_size = data_size,
        .count = 0,
        .size = new_size,
        .status = (key_status *)calloc(new_size, sizeof(key_status)),
        .keys = key_size != string_key_size ? (void *)calloc(new_size, key_size) : (void *)calloc(new_size, sizeof(char *)),
        .data = (void *)calloc(new_size, data_size),
        .hash_function = key_size != string_key_size ? (key_size <= sizeof(size_t) ? simple_hash : sdbm) : sdbm_str};
    return (unordered_map *)memcpy(malloc(sizeof(unordered_map)), &new_m, sizeof(unordered_map));
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
                if (*at_status(m, i) == USED)
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

static void expand_uo_map(unordered_map *const m)
{
    if (++m->count < m->size)
        return;
    size_t new_size = next_power_of_2(m->size << 1);
    key_status *new_status = (key_status *)calloc(new_size, sizeof(key_status));
    void *new_keys = m->key_size != string_key_size ? (void *)calloc(new_size, m->key_size) : (void *)calloc(new_size, sizeof(char *));
    void *new_data = (void *)calloc(new_size, m->data_size);
    size_t i = 0;
    for (; i < m->size; i++)
    {
        if (*at_status(m, i) != USED)
            continue;
        void *key = at_key(m, i);
        void *data = at_data(m, i);
        size_t index = m->hash_function((uint8_t *)key, m->key_size) % new_size;
        size_t offset = 1;
        for (;;)
        {
            key_status *status = at_status_p(new_status, index);
            if (*status != USED)
            {
                *status = USED;
                if (m->key_size == string_key_size)
                    memcpy(at_key_p(new_keys, index, m->key_size), key, sizeof(char *));
                else
                    memcpy(at_key_p(new_keys, index, m->key_size), key, m->key_size);
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

static void shrink_uo_map(unordered_map *const m)
{
    if (--m->count >= m->size >> 3)
        return;
    size_t new_size = next_power_of_2(m->size >> 1);
    if (new_size <= 4)
        return;
    key_status *new_status = (key_status *)calloc(new_size, sizeof(key_status));
    void *new_keys = m->key_size != string_key_size ? (void *)calloc(new_size, m->key_size) : (void *)calloc(new_size, sizeof(char *));
    void *new_data = (void *)calloc(new_size, m->data_size);
    size_t i = 0;
    for (; i < m->size; i++)
    {
        if (*at_status(m, i) != USED)
            continue;
        void *key = at_key(m, i);
        void *data = at_data(m, i);
        size_t index = m->hash_function((uint8_t *)key, m->key_size) % new_size;
        size_t offset = 1;
        for (;;)
        {
            key_status *status = at_status_p(new_status, index);
            if (*status != USED)
            {
                *status = USED;
                if (m->key_size == string_key_size)
                    memcpy(at_key_p(new_keys, index, m->key_size), key, sizeof(char *));
                else
                    memcpy(at_key_p(new_keys, index, m->key_size), key, m->key_size);
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

int set_uo_map(unordered_map *const m, const void *const key, const void *const val)
{
    size_t index = m->hash_function((uint8_t *)key, m->key_size) % m->size;
    size_t i = index;
    size_t offset = 1;
    for (;;)
    {
        key_status *status = at_status(m, i);
        if (*status == USED && key_cmp(at_key(m, i), key, m->key_size) == 0)
        {
            memcpy(at_data(m, i), val, m->data_size);
            return 1;
        }
        else
        {
            if (*status != USED)
            {
                *status = USED;
                key_cpy(at_key(m, i), key, m->key_size);
                memcpy(at_data(m, i), val, m->data_size);
                expand_uo_map(m);
                return 1;
            }
            else
            {
                i = (i + offset++) % m->size;
                if (i == index)
                    return 0;
            }
        }
    }
}

int get_uo_map(const unordered_map *const m, const void *const key, void *const val)
{
    if (m->count <= 0)
        return 0;
    size_t index = m->hash_function((uint8_t *)key, m->key_size) % m->size;
    size_t i = index;
    size_t offset = 1;
    for (;;)
    {
        key_status *status = at_status(m, i);
        if (*status == USED && key_cmp(at_key(m, i), key, m->key_size) == 0)
        {
            memcpy(val, at_data(m, i), m->data_size);
            return 1;
        }
        else
        {
            i = (i + offset++) % m->size;
            if (i == index)
                return 0;
        }
    }
}

int delete_uo_map(unordered_map *const m, const void *const key, void *const val)
{
    if (m->count <= 0)
        return 0;
    size_t index = m->hash_function((uint8_t *)key, m->key_size) % m->size;
    size_t i = index;
    size_t offset = 1;
    for (;;)
    {
        key_status *status = at_status(m, i);
        if (*status == USED && key_cmp(at_key(m, i), key, m->key_size) == 0)
        {
            if (val != nullptr)
                memcpy(val, at_data(m, i), m->data_size);
            *status = DELETED;
            if (m->key_size == string_key_size)
                free(*(char **)at_key(m, i));
            shrink_uo_map(m);
            return 1;
        }
        else
        {
            i = (i + offset++) % m->size;
            if (i == index)
                return 0;
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
        key_status *status = at_status(m, i);
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
