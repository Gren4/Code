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
    return m->keys + index * m->key_size;
}

static inline void *at_key_string(const unordered_map *const m, const size_t index)
{
    return m->keys + index * sizeof(char *);
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
    return keys + index * key_size;
}

static inline void *at_key_string_p(void *const keys, const size_t index)
{
    return keys + index * sizeof(char *);
}

static inline void *at_data_p(void *const data, const size_t index, const size_t data_size)
{
    return data + index * data_size;
}

static inline void keycpy_resize(void *const a, const void *const b, const size_t size)
{
    if (size > 0)
        memcpy(a, b, size);
    else
        memcpy(a, b, sizeof(char *));
    return;
}

static inline void keycpy(void *const a, const void *const b, const size_t size)
{
    if (size > 0)
        memcpy(a, b, size);
    else
    {
        char *string = strcpy(calloc(strlen(b) + 1, sizeof(char)), b);
        memcpy(a, &string, sizeof(char *));
    }
    return;
}

unordered_map create_uo_map(const size_t key_size, const size_t data_size)
{
    unordered_map new_m = {
        .key_size = key_size,
        .data_size = data_size,
        .count = 0,
        .size = 16,
        .status = (void *)calloc(16, sizeof(key_status)),
        .keys = key_size != string_key_size ? (void *)calloc(16, key_size) : (void *)calloc(16, sizeof(char *)),
        .data = (void *)calloc(16, data_size)};
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
                    free(*(char **)at_key_string(m, i));
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
        new_size = next_power_of_2(m->size << 2);
        new_status = (void *)calloc(new_size, sizeof(key_status));
        new_keys = m->key_size != string_key_size ? (void *)calloc(new_size, m->key_size) : (void *)calloc(new_size, sizeof(char *));
        new_data = (void *)calloc(new_size, m->data_size);
    }
    else
    {
        new_size = next_power_of_2(m->size >> 2);
        if (new_size <= 16)
            return;
        new_status = (void *)calloc(new_size, sizeof(key_status));
        new_keys = m->key_size != string_key_size ? (void *)calloc(new_size, m->key_size) : (void *)calloc(new_size, sizeof(char *));
        new_data = (void *)calloc(new_size, m->data_size);
    }
    if (m->key_size == string_key_size)
    {
        for (; i < m->size; i++)
        {
            if (*(key_status *)at_status(m, i) != USED)
                continue;
            void *key = at_key_string(m, i);
            void *data = at_data(m, i);
            size_t index = sdbm((uint8_t *)key, m->key_size) % new_size;
            size_t offset = 1;
            for (;;)
            {
                key_status *status = (key_status *)at_status_p(new_status, index);
                if (*status != USED)
                {
                    *status = USED;
                    memcpy(at_key_string_p(new_keys, index), key, sizeof(char *));
                    memcpy(at_data_p(new_data, index, m->data_size), data, m->data_size);
                    break;
                }
                else
                {
                    index = (index + offset++) % new_size;
                }
            }
        }
    }
    else
    {
        for (; i < m->size; i++)
        {
            if (*(key_status *)at_status(m, i) != USED)
                continue;
            void *key = at_key(m, i);
            void *data = at_data(m, i);
            size_t index = sdbm((uint8_t *)key, m->key_size) % new_size;
            size_t offset = 1;
            for (;;)
            {
                key_status *status = (key_status *)at_status_p(new_status, index);
                if (*status != USED)
                {
                    *status = USED;
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
    size_t index = sdbm((uint8_t *)key, m->key_size) % m->size;
    size_t i = index;
    size_t offset = 1;
    if (m->key_size == string_key_size)
    {
        for (;;)
        {
            key_status *status = (key_status *)at_status(m, i);
            if (*status == USED && strcmp(*(char **)at_key_string(m, i), key) == 0)
            {
                memcpy(at_data(m, i), val, m->data_size);
                return;
            }
            else
            {
                if (*status != USED)
                {
                    *status = USED;
                    char *string = strcpy(calloc(strlen(key) + 1, sizeof(char)), key);
                    memcpy(at_key_string(m, i), &string, sizeof(char *));
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
    }
    else
    {
        for (;;)
        {
            key_status *status = (key_status *)at_status(m, i);
            if (*status == USED && memcmp(at_key(m, i), key, m->key_size) == 0)
            {
                memcpy(at_data(m, i), val, m->data_size);
                return;
            }
            else
            {
                if (*status != USED)
                {
                    *status = USED;
                    memcpy(at_key(m, i), key, m->key_size);
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
    }
    return;
}

void get_uo_map(const unordered_map *const m, const void *const key, void *const val)
{
    size_t index = sdbm((uint8_t *)key, m->key_size) % m->size;
    size_t i = index;
    size_t offset = 1;
    if (m->key_size == string_key_size)
    {
        for (;;)
        {
            key_status *status = (key_status *)at_status(m, i);
            if (*status == USED && strcmp(*(char **)at_key_string(m, i), key) == 0)
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
    }
    else
    {
        for (;;)
        {
            key_status *status = (key_status *)at_status(m, i);
            if (*status == USED && memcmp(at_key(m, i), key, m->key_size) == 0)
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
    }
    return;
}

void delete_uo_map(unordered_map *const m, const void *const key, void *const val)
{
    size_t index = sdbm((uint8_t *)key, m->key_size) % m->size;
    size_t i = index;
    size_t offset = 1;
    if (m->key_size == string_key_size)
    {
        for (;;)
        {
            key_status *status = (key_status *)at_status(m, i);
            if (*status == USED && strcmp(*(char **)at_key_string(m, i), key) == 0)
            {
                if (val != nullptr)
                    memcpy(val, at_data(m, i), m->data_size);
                *status = DELETED;
                m->count--;
                free(*(char **)at_key_string(m, i));
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
    else
    {
        for (;;)
        {
            key_status *status = (key_status *)at_status(m, i);
            if (*status == USED && memcmp(at_key(m, i), key, m->key_size) == 0)
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
}

int has_key_uo_map(const unordered_map *const m, const void *const key)
{
    size_t index = sdbm((uint8_t *)key, m->key_size) % m->size;
    size_t i = index;
    size_t offset = 1;
    if (m->key_size == string_key_size)
    {
        for (;;)
        {
            key_status *status = (key_status *)at_status(m, i);
            if (*status == USED && strcmp(*(char **)at_key_string(m, i), key) == 0)
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
    else
    {
        for (;;)
        {
            key_status *status = (key_status *)at_status(m, i);
            if (*status == USED && memcmp(at_key(m, i), key, m->key_size) == 0)
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
}
