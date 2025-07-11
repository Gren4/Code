#include "hash_map.h"
#include "util_funcs.h"
#include <stdlib.h>
#include <string.h>

#define HASH_MAP_MIN_SIZE 32

typedef enum key_status
{
    EMPTY,
    USED,
    DELETED
} key_status;

typedef struct unordered_map
{
    size_t count;
    size_t size;
    key_status *status;
    const type_func *keys_type;
    const type_func *data_type;
    char *keys;
    char *data;
} unordered_map;

unordered_map *create_hash_map(const size_t size, const type_func *const keys_type, const type_func *const data_type)
{
    size_t new_size = size < HASH_MAP_MIN_SIZE ? HASH_MAP_MIN_SIZE : next_power_of_2(size);
    unordered_map new_m = {
        .count = 0,
        .size = new_size,
        .keys_type = keys_type,
        .data_type = data_type};
    new_m.status = (key_status *)calloc(new_size, sizeof(key_status));
    if (new_m.status == NULL)
        return NULL;
    new_m.keys = (char *)calloc(new_size, keys_type->t_size);
    if (new_m.keys == NULL)
    {
        free(new_m.status);
        return NULL;
    }
    new_m.data = (char *)calloc(new_size, data_type->t_size);
    if (new_m.data == NULL)
    {
        free(new_m.status);
        free(new_m.keys);
        return NULL;
    }
    return (unordered_map *)memcpy(malloc(sizeof(unordered_map)), &new_m, sizeof(unordered_map));
}

void free_hash_map(unordered_map *const m)
{
    if (m->keys != NULL)
    {
        if (m->keys_type->t_free != NULL && m->count > 0)
        {
            ssize_t i = 0;
            for (; i < m->size; i++)
            {
                if (m->status[i] == USED)
                    m->keys_type->t_free(m->keys_type->t_at(m->keys, i));
            }
        }
        free(m->keys);
    }
    if (m->data != NULL)
    {
        if (m->data_type->t_free != NULL && m->count > 0)
        {
            ssize_t i = 0;
            for (; i < m->size; i++)
            {
                if (m->status[i] == USED)
                    m->data_type->t_free(m->data_type->t_at(m->data, i));
            }
        }
        free(m->data);
    }
    if (m->status != NULL)
        free(m->status);
    m->count = 0;
    m->size = 0;
    m->status = NULL;
    m->keys = NULL;
    m->data = NULL;
    return;
}

static int expand_hash_map(unordered_map *const m)
{
    if (++m->count < m->size)
        return 1;
    size_t new_size = next_power_of_2(m->size << 1);
    key_status *new_status = (key_status *)calloc(new_size, sizeof(key_status));
    if (new_status == NULL)
        return 0;
    char *new_keys = (char *)calloc(new_size, m->keys_type->t_size);
    if (new_keys == NULL)
    {
        free(new_status);
        return 0;
    }
    char *new_data = (char *)calloc(new_size, m->data_type->t_size);
    if (new_data == NULL)
    {
        free(new_status);
        free(new_keys);
        return 0;
    }
    ssize_t i = 0;
    for (; i < m->size; i++)
    {
        if (m->status[i] != USED)
            continue;
        char *key = m->keys_type->t_at(m->keys, i);
        char *data = m->data_type->t_at(m->data, i);
        size_t index = m->keys_type->t_hash(m->keys_type->t_at(key, 0)) % new_size;
        size_t offset = 1;
        for (;;)
        {
            key_status *status = new_status + index;
            if (*status != USED)
            {
                *status = USED;
                m->keys_type->t_move(m->keys_type->t_at(new_keys, index), key);
                m->data_type->t_move(m->data_type->t_at(new_data, index), data);
                break;
            }
            index = (index + offset++) % new_size;
        }
    }
    free(m->status);
    free(m->keys);
    free(m->data);
    m->status = new_status;
    m->keys = new_keys;
    m->data = new_data;
    m->size = new_size;
    return 1;
}

static int shrink_hash_map(unordered_map *const m)
{
    if (--m->count > m->size >> 3)
        return 1;
    size_t new_size = next_power_of_2(m->size >> 1);
    if (new_size <= HASH_MAP_MIN_SIZE)
        return 1;
    key_status *new_status = (key_status *)calloc(new_size, sizeof(key_status));
    if (new_status == NULL)
        return 0;
    char *new_keys = (char *)calloc(new_size, m->keys_type->t_size);
    if (new_keys == NULL)
    {
        free(new_status);
        return 0;
    }
    char *new_data = (char *)calloc(new_size, m->data_type->t_size);
    if (new_data == NULL)
    {
        free(new_status);
        free(new_keys);
        return 0;
    }
    ssize_t i = 0;
    for (; i < m->size; i++)
    {
        if (m->status[i] != USED)
            continue;
        char *key = m->keys_type->t_at(m->keys, i);
        char *data = m->data_type->t_at(m->data, i);
        size_t index = m->keys_type->t_hash(m->keys_type->t_at(key, 0)) % new_size;
        size_t offset = 1;
        for (;;)
        {
            key_status *status = new_status + index;
            if (*status != USED)
            {
                *status = USED;
                m->keys_type->t_move(m->keys_type->t_at(new_keys, index), key);
                m->data_type->t_move(m->data_type->t_at(new_data, index), data);
                break;
            }
            index = (index + offset++) % new_size;
        }
    }
    free(m->status);
    free(m->keys);
    free(m->data);
    m->status = new_status;
    m->keys = new_keys;
    m->data = new_data;
    m->size = new_size;
    return 1;
}

int set_hash_map(unordered_map *const m, const void *const key, const void *const val)
{
    if (key == NULL || val == NULL || m->count == SIZE_MAX)
        return 0;
    size_t index = m->keys_type->t_hash(m->keys_type->t_at(key, 0)) % m->size;
    size_t i = index;
    size_t offset = 1;
    for (;;)
    {
        key_status *status = m->status + i;
        if (*status == USED && m->keys_type->t_cmp(m->keys_type->t_at(m->keys, i), key) == 0)
        {
            m->data_type->t_cpy(m->data_type->t_at(m->data, i), val);
            return 1;
        }
        else
        {
            if (*status != USED)
            {
                *status = USED;
                m->keys_type->t_cpy(m->keys_type->t_at(m->keys, i), key);
                m->data_type->t_cpy(m->data_type->t_at(m->data, i), val);
                return expand_hash_map(m);
            }
            i = (i + offset++) % m->size;
            if (i == index)
                return 0;
        }
    }
}

static size_t get_index_hash_map(const unordered_map *const m, const void *const key)
{
    size_t index = m->keys_type->t_hash(m->keys_type->t_at(key, 0)) % m->size;
    size_t i = index;
    size_t offset = 1;
    for (;;)
    {
        key_status *status = m->status + i;
        if (*status == USED && m->keys_type->t_cmp(m->keys_type->t_at(m->keys, i), key) == 0)
            return i;
        else if (*status == EMPTY)
            return m->count;
        i = (i + offset++) % m->size;
        if (i == index)
            return m->count;
    }
}

int get_hash_map(const unordered_map *const m, const void *const key, void *const val)
{
    if (key == NULL || val == NULL || m->count == 0)
        return 0;
    size_t index = get_index_hash_map(m, key);
    if (index == m->count)
        return 0;
    m->data_type->t_cpy(val, m->data_type->t_at(m->data, index));
    return 1;
}

int delete_hash_map(unordered_map *const m, const void *const key, void *const val)
{
    if (key == NULL || m->count == 0)
        return 0;
    size_t index = get_index_hash_map(m, key);
    if (index == m->count)
        return 0;
    if (val != NULL)
        m->data_type->t_cpy(val, m->data_type->t_at(m->data, index));
    *(m->status + index) = DELETED;
    if (m->keys_type->t_free != NULL)
        m->keys_type->t_free(m->keys_type->t_at(m->keys, index));
    if (m->data_type->t_free != NULL)
        m->data_type->t_free(m->data_type->t_at(m->data, index));
    return shrink_hash_map(m);
}

int has_key_hash_map(const unordered_map *const m, const void *const key)
{
    if (key == NULL)
        return 0;
    else
        return get_index_hash_map(m, key) != m->count;
}
