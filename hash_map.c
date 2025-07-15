#include "hash_map.h"
#include "shared_ptr.h"
#include "util_funcs.h"
#include <stdlib.h>
#include <string.h>

#define HASH_MAP_MIN_SIZE 32

typedef enum key_status
{
    EMPTY,
    DELETED,
    USED_1,
    USED_2
} key_status;

typedef struct unordered_map
{
    size_t count;
    size_t size;
    const type_func *keys_type;
    const type_func *container_type;
    shared_ptr status;
    shared_ptr keys;
    shared_ptr container;
    key_status cur_flag;
} unordered_map;

unordered_map *create_hash_map(const size_t size, const type_func *const keys_type, const type_func *const container_type)
{
    size_t mul_of_2_size = size < HASH_MAP_MIN_SIZE ? HASH_MAP_MIN_SIZE : next_power_of_2(size);
    unordered_map new_m = {
        .count = 0,
        .size = mul_of_2_size,
        .keys_type = keys_type,
        .container_type = container_type,
        .cur_flag = USED_1};
    new_m.status = calloc_shared_ptr(mul_of_2_size, sizeof(key_status));
    if (new_m.status == NULL)
        return NULL;
    new_m.keys = calloc_shared_ptr(mul_of_2_size, keys_type->t_size);
    if (new_m.keys == NULL)
    {
        free_shared_ptr(new_m.status);
        return NULL;
    }
    new_m.container = calloc_shared_ptr(mul_of_2_size, container_type->t_size);
    if (new_m.container == NULL)
    {
        free_shared_ptr(new_m.status);
        free_shared_ptr(new_m.keys);
        return NULL;
    }
    return (unordered_map *)memcpy(malloc(sizeof(unordered_map)), &new_m, sizeof(unordered_map));
}

void free_hash_map(unordered_map *const m)
{
    if (m->keys != NULL)
    {
        ssize_t i = 0;
        key_status *status_data = data_shared_ptr(m->status);
        void *keys_data = data_shared_ptr(m->keys);
        for (; i < m->size; i++)
        {
            if (status_data[i] == m->cur_flag)
                m->keys_type->t_free(m->keys_type->t_at(keys_data, i));
        }
        free_shared_ptr(m->keys);
    }
    if (m->container != NULL)
    {
        ssize_t i = 0;
        key_status *status_data = data_shared_ptr(m->status);
        void *container_data = data_shared_ptr(m->container);
        for (; i < m->size; i++)
        {
            if (status_data[i] == m->cur_flag)
                m->container_type->t_free(m->container_type->t_at(container_data, i));
        }
        free_shared_ptr(m->container);
    }
    if (m->status != NULL)
        free_shared_ptr(m->status);
    m->count = 0;
    m->size = 0;
    m->status = NULL;
    m->keys = NULL;
    m->container = NULL;
    free(m);
    return;
}

static int expand_hash_map(unordered_map *const m)
{
    if (++m->count < m->size)
        return 1;
    size_t mul_of_2_size = next_power_of_2(m->size << 1);
    shared_ptr new_status = realloc_shared_ptr(m->status, mul_of_2_size, sizeof(key_status));
    if (new_status == NULL)
        return 0;
    shared_ptr new_keys = realloc_shared_ptr(m->keys, mul_of_2_size, m->keys_type->t_size);
    if (new_keys == NULL)
    {
        free_shared_ptr(new_status);
        return 0;
    }
    shared_ptr new_container = realloc_shared_ptr(m->container, mul_of_2_size, m->container_type->t_size);
    if (new_container == NULL)
    {
        free_shared_ptr(new_status);
        free_shared_ptr(new_keys);
        return 0;
    }
    ssize_t i = 0;
    key_status new_flag = m->cur_flag == USED_1 ? USED_2 : USED_1;
    key_status *status_data = data_shared_ptr(new_status);
    void *keys_data = data_shared_ptr(new_keys);
    void *container_data = data_shared_ptr(new_container);
    memset(status_data + m->size, 0, (mul_of_2_size - m->size) * sizeof(key_status));
    memset(m->keys_type->t_at(keys_data, m->size), 0, (mul_of_2_size - m->size) * m->keys_type->t_size);
    memset(m->container_type->t_at(container_data, m->size), 0, (mul_of_2_size - m->size) * m->container_type->t_size);
    for (; i < m->size; i++)
    {
        if (status_data[i] != m->cur_flag)
            continue;
        key_status *cur_status = status_data + i;
        void *key = m->keys_type->t_at(keys_data, i);
        void *val = m->container_type->t_at(container_data, i);
        size_t index = m->keys_type->t_hash(key) % mul_of_2_size;
        size_t offset = 1;
        for (;;)
        {
            key_status *old_status = status_data + index;
            if (*old_status != new_flag)
            {
                if (*old_status == m->cur_flag)
                    i--;
                m->keys_type->t_swap(m->keys_type->t_at(keys_data, index), key);
                m->container_type->t_swap(m->container_type->t_at(container_data, index), val);
                *cur_status = (*old_status == DELETED) ? EMPTY : *old_status;
                *old_status = new_flag;
                break;
            }
            index = (index + offset++) % mul_of_2_size;
        }
    }
    m->status = new_status;
    m->keys = new_keys;
    m->container = new_container;
    m->size = mul_of_2_size;
    m->cur_flag = new_flag;
    return 1;
}

static int shrink_hash_map(unordered_map *const m)
{
    if (--m->count > m->size >> 3)
        return 1;
    size_t mul_of_2_size = next_power_of_2(m->count);
    if (mul_of_2_size <= HASH_MAP_MIN_SIZE)
        return 1;
    ssize_t i = 0;
    key_status new_flag = m->cur_flag == USED_1 ? USED_2 : USED_1;
    key_status *status_data = data_shared_ptr(m->status);
    void *keys_data = data_shared_ptr(m->keys);
    void *container_data = data_shared_ptr(m->container);
    for (; i < m->size; i++)
    {
        if (status_data[i] != m->cur_flag)
            continue;
        key_status *cur_status = status_data + i;
        void *key = m->keys_type->t_at(keys_data, i);
        void *val = m->container_type->t_at(container_data, i);
        size_t index = m->keys_type->t_hash(key) % mul_of_2_size;
        size_t offset = 1;
        for (;;)
        {
            key_status *old_status = status_data + index;
            if (*old_status != new_flag)
            {
                if (*old_status == m->cur_flag)
                    i--;
                m->keys_type->t_swap(m->keys_type->t_at(keys_data, index), key);
                m->container_type->t_swap(m->container_type->t_at(container_data, index), val);
                *cur_status = (*old_status == DELETED) ? EMPTY : *old_status;
                *old_status = new_flag;
                break;
            }
            index = (index + offset++) % mul_of_2_size;
        }
    }
    shared_ptr new_status = realloc_shared_ptr(m->status, mul_of_2_size, sizeof(key_status));
    if (new_status == NULL)
        return 0;
    shared_ptr new_keys = realloc_shared_ptr(m->keys, mul_of_2_size, m->keys_type->t_size);
    if (new_keys == NULL)
    {
        free_shared_ptr(new_status);
        return 0;
    }
    shared_ptr new_container = realloc_shared_ptr(m->container, mul_of_2_size, m->container_type->t_size);
    if (new_container == NULL)
    {
        free_shared_ptr(new_status);
        free_shared_ptr(new_keys);
        return 0;
    }
    m->status = new_status;
    m->keys = new_keys;
    m->container = new_container;
    m->size = mul_of_2_size;
    m->cur_flag = new_flag;
    return 1;
}

int set_hash_map(unordered_map *const m, const void *const key, const void *const val)
{
    if (key == NULL || val == NULL || m->count == SIZE_MAX)
        return 0;
    size_t index = m->keys_type->t_hash(key) % m->size;
    size_t i = index;
    size_t offset = 1;
    key_status *status_data = data_shared_ptr(m->status);
    void *keys_data = data_shared_ptr(m->keys);
    void *container_data = data_shared_ptr(m->container);
    for (;;)
    {
        key_status *status = status_data + i;
        if (*status == m->cur_flag && m->keys_type->t_cmp(m->keys_type->t_at(keys_data, i), key) == 0)
        {
            m->container_type->t_free(m->container_type->t_at(container_data, i));
            m->container_type->t_cpy(m->container_type->t_at(container_data, i), val);
            return 1;
        }
        else
        {
            if (*status != m->cur_flag)
            {
                *status = m->cur_flag;
                m->keys_type->t_cpy(m->keys_type->t_at(keys_data, i), key);
                m->container_type->t_cpy(m->container_type->t_at(container_data, i), val);
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
    size_t index = m->keys_type->t_hash(key) % m->size;
    size_t i = index;
    size_t offset = 1;
    key_status *status_data = data_shared_ptr(m->status);
    void *keys_data = data_shared_ptr(m->keys);
    for (;;)
    {
        key_status *status = status_data + i;
        if (*status == m->cur_flag && m->keys_type->t_cmp(m->keys_type->t_at(keys_data, i), key) == 0)
            return i;
        else if (*status == EMPTY)
            return m->size;
        i = (i + offset++) % m->size;
        if (i == index)
            return m->size;
    }
}

int get_hash_map(const unordered_map *const m, const void *const key, void *const val)
{
    if (key == NULL || val == NULL || m->count == 0)
        return 0;
    size_t index = get_index_hash_map(m, key);
    if (index == m->size)
        return 0;
    m->container_type->t_cpy(val, m->container_type->t_at(data_shared_ptr(m->container), index));
    return 1;
}

int delete_hash_map(unordered_map *const m, const void *const key, void *const val)
{
    if (key == NULL || m->count == 0)
        return 0;
    size_t index = get_index_hash_map(m, key);
    if (index == m->size)
        return 0;
    void *container_data = data_shared_ptr(m->container);
    if (val != NULL)
        m->container_type->t_cpy(val, m->container_type->t_at(container_data, index));
    key_status *status_data = data_shared_ptr(m->status);
    *(status_data + index) = DELETED;
    m->keys_type->t_free(m->keys_type->t_at(data_shared_ptr(m->keys), index));
    m->container_type->t_free(m->container_type->t_at(container_data, index));
    return shrink_hash_map(m);
}

int has_key_hash_map(const unordered_map *const m, const void *const key)
{
    if (key == NULL)
        return 0;
    else
        return get_index_hash_map(m, key) != m->size;
}
