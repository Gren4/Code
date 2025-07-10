#pragma once

#include "type_functionality.h"

typedef struct rbt_map rbt_map;

rbt_map *create_rbt_map(const size_t size, const type_func *keys_type, const type_func *data_type);
size_t has_rbt_map(const rbt_map *const m, const void *const key);
int set_rbt_map(rbt_map *const m, const void *const key, const void *const val);
int delete_rbt_map(rbt_map *m, const void *const key, void *const val);
void free_rbt_map(rbt_map *tree);
