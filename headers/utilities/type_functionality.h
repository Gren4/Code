#pragma once

#include <stdint.h>

typedef struct type_func
{
    size_t t_size;
    void *(*t_at)(const void *const, const size_t);
    int (*t_cmp)(const void *const, const void *const);
    void *(*t_cpy)(void *const, const void *const);
    void *(*t_move)(void *const, const void *const);
    void (*t_free)(void *const);
    size_t (*t_hash)(const void *const);
    void (*t_swap)(void *const, void *const);
} type_func;

extern const type_func *f_uint8_t;
extern const type_func *f_int8_t;
extern const type_func *f_uint16_t;
extern const type_func *f_int16_t;
extern const type_func *f_uint32_t;
extern const type_func *f_int32_t;
extern const type_func *f_uint64_t;
extern const type_func *f_int64_t;
extern const type_func *f_size_t;
extern const type_func *f_ssize_t;
extern const type_func *f_string_t;
extern const type_func *f_float;
extern const type_func *f_double;
extern const type_func *f_vector;
extern const type_func *f_dequeue;
extern const type_func *f_queue;
extern const type_func *f_hash_map;
extern const type_func *f_rbt_map;