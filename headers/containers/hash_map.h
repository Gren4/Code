#pragma once

#include "type_functionality.h"
#include "shared_ptr.h"

/* Type declaration */
typedef shared_ptr hash_map;

/* Main function's declarations */
hash_map create_hash_map(const size_t size, const type_func *const keys_type, const type_func *const container_type);
void free_hash_map(hash_map const m);
int set_hash_map(hash_map const m, const void *const key, const void *const val);
int get_hash_map(const hash_map m, const void *const key, void *const val);
int delete_hash_map(hash_map const m, const void *const key, void *const val);
int has_key_hash_map(const hash_map m, const void *const key);
