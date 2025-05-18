#pragma once

#include <stdint.h>

#define string_key_size 0

typedef struct map map;

map *create_map(const size_t key_size, const size_t data_size);
void free_map(map *const m);
int set_map(map *const m, const void *const key, const void *const val);
int delete_map(map *const m, const void *const key, void *const val);