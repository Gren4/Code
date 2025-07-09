#include "unordered_map.h"
#include "util_funcs.h"
#include <stdlib.h>
#include <string.h>

#define UO_MAP_MIN_SIZE 16

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
    char *keys;
    char *data;
    const type_func *keys_type;
    const type_func *data_type;
} unordered_map;

unordered_map *create_uo_map(const size_t size, const type_func *keys_type, const type_func *data_type)
{
    size_t new_size = size < UO_MAP_MIN_SIZE ? UO_MAP_MIN_SIZE : next_power_of_2(size);
    unordered_map new_m = {
        .count = 0,
        .size = new_size,
        .status = (key_status *)calloc(new_size, sizeof(key_status)),
        .keys = (char *)calloc(new_size, keys_type->t_size),
        .data = (char *)calloc(new_size, data_type->t_size),
        .keys_type = keys_type,
        .data_type = data_type};
    return (unordered_map *)memcpy(malloc(sizeof(unordered_map)), &new_m, sizeof(unordered_map));
}

void free_uo_map(unordered_map *const m)
{
    if (m->keys != NULL)
    {
        if (m->keys_type->t_free != NULL && m->count > 0)
        {
            size_t i = 0;
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
            size_t i = 0;
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

static int expand_uo_map(unordered_map *const m)
{
    if (++m->count < m->size)
        return 1;
    size_t new_size = next_power_of_2(m->size << 1);
    key_status *new_status = (key_status *)calloc(new_size, sizeof(key_status));
    char *new_keys = (char *)calloc(new_size, m->keys_type->t_size);
    char *new_data = (char *)calloc(new_size, m->data_type->t_size);
    if (new_status == NULL || new_keys == NULL || new_data == NULL)
        return 0;
    size_t i = 0;
    for (; i < m->size; i++)
    {
        if (m->status[i] != USED)
            continue;
        char *key = m->keys_type->t_at(m->keys, i);
        char *data = m->data_type->t_at(m->data, i);
        size_t index = m->keys_type->t_hash(key) % new_size;
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
    return 1;
}

static int shrink_uo_map(unordered_map *const m)
{
    if (--m->count >= m->size >> 3)
        return 1;
    size_t new_size = next_power_of_2(m->size >> 1);
    if (new_size <= UO_MAP_MIN_SIZE)
        return 1;
    key_status *new_status = (key_status *)calloc(new_size, sizeof(key_status));
    char *new_keys = (char *)calloc(new_size, m->keys_type->t_size);
    char *new_data = (char *)calloc(new_size, m->data_type->t_size);
    if (new_status == NULL || new_keys == NULL || new_data == NULL)
        return 0;
    size_t i = 0;
    for (; i < m->size; i++)
    {
        if (m->status[i] != USED)
            continue;
        char *key = m->keys_type->t_at(m->keys, i);
        char *data = m->data_type->t_at(m->data, i);
        size_t index = m->keys_type->t_hash(key) % new_size;
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
    return 1;
}

int set_uo_map(unordered_map *const m, const void *const key, const void *const val)
{
    size_t index = m->keys_type->t_hash(key) % m->size;
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
                return expand_uo_map(m);
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
    size_t index = m->keys_type->t_hash(key) % m->size;
    size_t i = index;
    size_t offset = 1;
    for (;;)
    {
        key_status *status = m->status + i;
        if (*status == USED && m->keys_type->t_cmp(m->keys_type->t_at(m->keys, i), key) == 0)
        {
            m->data_type->t_cpy(val, m->data_type->t_at(m->data, i));
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
    size_t index = m->keys_type->t_hash(key) % m->size;
    size_t i = index;
    size_t offset = 1;
    for (;;)
    {
        key_status *status = m->status + i;
        if (*status == USED && m->keys_type->t_cmp(m->keys_type->t_at(m->keys, i), key) == 0)
        {
            if (val != NULL)
                m->data_type->t_cpy(val, m->data_type->t_at(m->data, i));
            *status = DELETED;
            if (m->keys_type->t_free != NULL)
                m->keys_type->t_free(m->keys_type->t_at(m->keys, i));
            if (m->data_type->t_free != NULL)
                m->data_type->t_free(m->data_type->t_at(m->data, i));
            return shrink_uo_map(m);
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
    size_t index = m->keys_type->t_hash(key) % m->size;
    size_t i = index;
    size_t offset = 1;
    for (;;)
    {
        key_status *status = m->status + i;
        if (*status == USED && m->keys_type->t_cmp(m->keys_type->t_at(m->keys, i), key) == 0)
            return 1;
        else if (*status == EMPTY)
            return 0;
        else
        {
            i = (i + offset++) % m->size;
            if (i == index)
                return 0;
        }
    }
}
