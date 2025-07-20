#include "hash_map.h"
#include "util_funcs.h"
#include <stdlib.h>
#include <string.h>

#define HASH_MAP_MIN_SIZE 32

/* Struct's declarations */
typedef enum key_status
{
    EMPTY,
    DELETED,
    USED_1,
    USED_2
} key_status;

typedef struct hash_map_t
{
    size_t count;
    size_t size;
    const type_func *keys_type;
    const type_func *container_type;
    key_status *status;
    void *keys;
    void *container;
    key_status cur_flag;
} hash_map_t;

/* Static function's declarations */
static int expand_hash_map(hash_map_t *const m);
static int shrink_hash_map(hash_map_t *const m);
static size_t get_index_hash_map(const hash_map_t *const m, const void *const key);

/* Main functions */
hash_map create_hash_map(const size_t size, const type_func *const keys_type, const type_func *const container_type)
{
    hash_map ptr = malloc_shared_ptr(1, sizeof(hash_map_t));
    if (ptr == NULL)
        return NULL;
    size_t mul_of_2_size = size < HASH_MAP_MIN_SIZE ? HASH_MAP_MIN_SIZE : next_power_of_2(size);
    hash_map_t new_m = {
        .count = 0,
        .size = mul_of_2_size,
        .keys_type = keys_type,
        .container_type = container_type,
        .cur_flag = USED_1};
    new_m.status = calloc(mul_of_2_size, sizeof(key_status));
    if (new_m.status == NULL)
    {
        free_shared_ptr(ptr);
        return NULL;
    }
    new_m.keys = calloc(mul_of_2_size, keys_type->t_size);
    if (new_m.keys == NULL)
    {
        free_shared_ptr(ptr);
        free(new_m.status);
        return NULL;
    }
    new_m.container = calloc(mul_of_2_size, container_type->t_size);
    if (new_m.container == NULL)
    {
        free_shared_ptr(ptr);
        free(new_m.status);
        free(new_m.keys);
        return NULL;
    }
    memcpy(data_shared_ptr(ptr), &new_m, sizeof(hash_map_t));
    return ptr;
}

void free_hash_map(hash_map const ptr)
{
    if (ptr == NULL)
        return;
    if (count_shared_ptr(ptr) == 1)
    {
        hash_map_t *m = data_shared_ptr(ptr);
        if (m->keys != NULL)
        {
            size_t i = 0;
            for (; i < m->size; i++)
            {
                if (m->status[i] == m->cur_flag)
                    m->keys_type->t_free(m->keys_type->t_at(m->keys, i));
            }
            free(m->keys);
        }
        if (m->container != NULL)
        {
            size_t i = 0;
            for (; i < m->size; i++)
            {
                if (m->status[i] == m->cur_flag)
                    m->container_type->t_free(m->container_type->t_at(m->container, i));
            }
            free(m->container);
        }
        if (m->status != NULL)
            free(m->status);
        m->count = 0;
        m->size = 0;
        m->status = NULL;
        m->keys = NULL;
        m->container = NULL;
    }
    free_shared_ptr(ptr);
    return;
}

int set_hash_map(hash_map const ptr, const void *const key, const void *const val)
{
    if (ptr == NULL)
        return 0;
    hash_map_t *m = data_shared_ptr(ptr);
    if (key == NULL || val == NULL || m->count == SIZE_MAX)
        return 0;
    size_t index = m->keys_type->t_hash(key) % m->size;
    size_t i = index;
    size_t offset = 1;
    for (;;)
    {
        key_status *status = m->status + i;
        if (*status == m->cur_flag && m->keys_type->t_cmp(m->keys_type->t_at(m->keys, i), key) == 0)
        {
            m->container_type->t_free(m->container_type->t_at(m->container, i));
            m->container_type->t_cpy(m->container_type->t_at(m->container, i), val);
            return 1;
        }
        else
        {
            if (*status != m->cur_flag)
            {
                *status = m->cur_flag;
                m->keys_type->t_cpy(m->keys_type->t_at(m->keys, i), key);
                m->container_type->t_cpy(m->container_type->t_at(m->container, i), val);
                return expand_hash_map(m);
            }
            i = (i + offset++) % m->size;
            if (i == index)
                return 0;
        }
    }
}

int get_hash_map(const hash_map ptr, const void *const key, void *const val)
{
    if (ptr == NULL)
        return 0;
    hash_map_t *m = data_shared_ptr(ptr);
    if (key == NULL || val == NULL || m->count == 0)
        return 0;
    size_t index = get_index_hash_map(m, key);
    if (index == m->size)
        return 0;
    m->container_type->t_cpy(val, m->container_type->t_at(m->container, index));
    return 1;
}

int delete_hash_map(hash_map const ptr, const void *const key, void *const val)
{
    if (ptr == NULL)
        return 0;
    hash_map_t *m = data_shared_ptr(ptr);
    if (key == NULL || m->count == 0)
        return 0;
    size_t index = get_index_hash_map(m, key);
    if (index == m->size)
        return 0;
    if (val != NULL)
        m->container_type->t_cpy(val, m->container_type->t_at(m->container, index));
    *(m->status + index) = DELETED;
    m->keys_type->t_free(m->keys_type->t_at(m->keys, index));
    m->container_type->t_free(m->container_type->t_at(m->container, index));
    return shrink_hash_map(m);
}

int has_key_hash_map(const hash_map ptr, const void *const key)
{
    if (ptr == NULL)
        return 0;
    hash_map_t *m = data_shared_ptr(ptr);
    if (key == NULL)
        return 0;
    else
        return get_index_hash_map(m, key) != m->size;
}

/* Static functions */
static int expand_hash_map(hash_map_t *const m)
{
    if (++m->count < m->size)
        return 1;
    size_t mul_of_2_size = next_power_of_2(m->size << 1);
    key_status *new_status = realloc(m->status, mul_of_2_size * sizeof(key_status));
    if (new_status == NULL)
        return 0;
    void *new_keys = realloc(m->keys, mul_of_2_size * m->keys_type->t_size);
    if (new_keys == NULL)
    {
        free(new_status);
        return 0;
    }
    void *new_container = realloc(m->container, mul_of_2_size * m->container_type->t_size);
    if (new_container == NULL)
    {
        free(new_status);
        free(new_keys);
        return 0;
    }
    size_t i = 0;
    key_status new_flag = m->cur_flag == USED_1 ? USED_2 : USED_1;
    memset(new_status + m->size, 0, (mul_of_2_size - m->size) * sizeof(key_status));
    memset(m->keys_type->t_at(new_keys, m->size), 0, (mul_of_2_size - m->size) * m->keys_type->t_size);
    memset(m->container_type->t_at(new_container, m->size), 0, (mul_of_2_size - m->size) * m->container_type->t_size);
    for (; i < m->size; i++)
    {
        if (new_status[i] != m->cur_flag)
            continue;
        key_status *cur_status = new_status + i;
        void *key = m->keys_type->t_at(new_keys, i);
        void *val = m->container_type->t_at(new_container, i);
        size_t index = m->keys_type->t_hash(key) % mul_of_2_size;
        size_t offset = 1;
        for (;;)
        {
            key_status *old_status = new_status + index;
            if (*old_status != new_flag)
            {
                if (*old_status == m->cur_flag)
                    i--;
                m->keys_type->t_swap(m->keys_type->t_at(new_keys, index), key);
                m->container_type->t_swap(m->container_type->t_at(new_container, index), val);
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

static int shrink_hash_map(hash_map_t *const m)
{
    if (--m->count > m->size >> 3)
        return 1;
    size_t mul_of_2_size = next_power_of_2(m->count);
    if (mul_of_2_size <= HASH_MAP_MIN_SIZE)
        return 1;
    size_t i = 0;
    key_status new_flag = m->cur_flag == USED_1 ? USED_2 : USED_1;
    for (; i < m->size; i++)
    {
        if (m->status[i] != m->cur_flag)
            continue;
        key_status *cur_status = m->status + i;
        void *key = m->keys_type->t_at(m->keys, i);
        void *val = m->container_type->t_at(m->container, i);
        size_t index = m->keys_type->t_hash(key) % mul_of_2_size;
        size_t offset = 1;
        for (;;)
        {
            key_status *old_status = m->status + index;
            if (*old_status != new_flag)
            {
                if (*old_status == m->cur_flag)
                    i--;
                m->keys_type->t_swap(m->keys_type->t_at(m->keys, index), key);
                m->container_type->t_swap(m->container_type->t_at(m->container, index), val);
                *cur_status = (*old_status == DELETED) ? EMPTY : *old_status;
                *old_status = new_flag;
                break;
            }
            index = (index + offset++) % mul_of_2_size;
        }
    }
    key_status *new_status = realloc(m->status, mul_of_2_size * sizeof(key_status));
    if (new_status == NULL)
        return 0;
    void *new_keys = realloc(m->keys, mul_of_2_size * m->keys_type->t_size);
    if (new_keys == NULL)
    {
        free(new_status);
        return 0;
    }
    void *new_container = realloc(m->container, mul_of_2_size * m->container_type->t_size);
    if (new_container == NULL)
    {
        free(new_status);
        free(new_keys);
        return 0;
    }
    m->status = new_status;
    m->keys = new_keys;
    m->container = new_container;
    m->size = mul_of_2_size;
    m->cur_flag = new_flag;
    return 1;
}

static size_t get_index_hash_map(const hash_map_t *const m, const void *const key)
{
    size_t index = m->keys_type->t_hash(key) % m->size;
    size_t i = index;
    size_t offset = 1;
    for (;;)
    {
        key_status *status = m->status + i;
        if (*status == m->cur_flag && m->keys_type->t_cmp(m->keys_type->t_at(m->keys, i), key) == 0)
            return i;
        else if (*status == EMPTY)
            return m->size;
        i = (i + offset++) % m->size;
        if (i == index)
            return m->size;
    }
}

/* Type functionality */
void *t_at_hash_map(const void *const src, const size_t index)
{
    return ((hash_map *)src) + index;
}

int t_cmp_hash_map(const void *const src_1, const void *const src_2)
{
    hash_map_t *a = data_shared_ptr(*(hash_map *)src_1);
    hash_map_t *b = data_shared_ptr(*(hash_map *)src_2);
    return a->size > b->size;
}

void *t_cpy_hash_map(void *const dest, const void *const src)
{
    if (*(hash_map *)dest != *(hash_map *)src)
        *(hash_map *)dest = copy_shared_ptr(*(hash_map *)src);
    return dest;
}

void *t_move_hash_map(void *const dest, const void *const src)
{
    *(hash_map *)dest = *(hash_map *)src;
    return dest;
}

void t_free_hash_map(void *const src)
{
    hash_map *val = (hash_map *)src;
    free_hash_map(*val);
    return;
}

size_t t_hash_hash_map(const void *const src)
{
    hash_map_t *m = data_shared_ptr(*(hash_map *)src);
    size_t h = 0;
    h = hash_size_t(m->count);
    h = hash_size_t(m->size) ^ (h << 6) ^ (h >> 2);
    h = hash_size_t((size_t)m->keys_type) ^ (h << 6) ^ (h >> 2);
    h = hash_size_t((size_t)m->container_type) ^ (h << 6) ^ (h >> 2);
    h = hash_size_t((size_t)m->status) ^ (h << 6) ^ (h >> 2);
    h = hash_size_t((size_t)m->keys) ^ (h << 6) ^ (h >> 2);
    h = hash_size_t((size_t)m->container) ^ (h << 6) ^ (h >> 2);
    h = hash_size_t((size_t)m->cur_flag) ^ (h << 6) ^ (h >> 2);
    return h;
}

void t_swap_hash_map(void *const src_1, void *const src_2)
{
    if (src_1 == src_2)
        return;
    size_t *a = (size_t *)src_1;
    size_t *b = (size_t *)src_2;
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
    return;
}

const type_func orig_f_hash_map = {
    .t_size = sizeof(hash_map),
    .t_at = t_at_hash_map,
    .t_cmp = t_cmp_hash_map,
    .t_cpy = t_cpy_hash_map,
    .t_move = t_move_hash_map,
    .t_free = t_free_hash_map,
    .t_hash = t_hash_hash_map,
    .t_swap = t_swap_hash_map};

const type_func *f_hash_map = &orig_f_hash_map;
