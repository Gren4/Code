#pragma once

#include "type_functionality.h"

typedef struct unordered_map unordered_map;

unordered_map *create_uo_map(const size_t size, const type_func *keys_type, const type_func *data_type);
void free_uo_map(unordered_map *const m);
int set_uo_map(unordered_map *const m, const void *const key, const void *const val);
int get_uo_map(const unordered_map *const m, const void *const key, void *const val);
int delete_uo_map(unordered_map *const m, const void *const key, void *const val);
int has_key_uo_map(const unordered_map *const m, const void *const key);
