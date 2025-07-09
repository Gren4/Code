#pragma once

#include <stdint.h>

typedef struct type_func
{
    size_t t_size;
    void *(*t_at)(const void *, const size_t);
    int (*t_cmp)(const void *, const void *);
    void *(*t_cpy)(void *, const void *);
    void *(*t_move)(void *, const void *);
    void (*t_free)(void *);
    size_t (*t_hash)(const void *);
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
extern const type_func *f_string;
extern const type_func *f_float;
extern const type_func *f_double;