#pragma once

#include <stdint.h>

#define string_key_size -1

typedef enum resize_type
{
    EXPAND,
    SHRINK
} resize_type;

typedef struct unordered_map
{
    size_t key_size;
    size_t data_size;
    size_t count;
    size_t size;
    void *status;
    void *keys;
    void *data;
} unordered_map;

unordered_map create_uo_map(const size_t key_size, const size_t data_size);
void free_uo_map(unordered_map *const m);
void resize_uo_map(unordered_map *const m, resize_type type);
void set_uo_map(unordered_map *const m, const void *const key, const void *const val);
void get_uo_map(unordered_map *const m, const void *const key, void *const val);
void delete_uo_map(unordered_map *const m, const void *const key, void *const val);
int has_key_uo_map(unordered_map *const m, const void *const key);

