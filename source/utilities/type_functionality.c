#include "type_functionality.h"
#include "string_type.h"
#include "util_funcs.h"
#include <stdint.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <string.h>

/* Hash functions */
size_t t_hash_8bit(const void *const src)
{
    return hash_8bit(*(uint8_t *)src);
}

size_t t_hash_16bit(const void *const src)
{
    return hash_16bit(*(uint16_t *)src);
}

size_t t_hash_32bit(const void *const src)
{
    return hash_32bit(*(uint32_t *)src);
}

size_t t_hash_64bit(const void *const src)
{
    return hash_64bit(*(uint64_t *)src);
}

size_t t_hash_size_t(const void *const src)
{
    return hash_size_t(*(size_t *)src);
}

size_t t_hash_string_t(const void *const src)
{
    return hash_string_t(get_string(*(string_t *)src));
}

/* Swap functions */
void t_swap_8bit(void *const src_1, void *const src_2)
{
    if (src_1 == src_2)
        return;
    uint8_t *a = (uint8_t *)src_1;
    uint8_t *b = (uint8_t *)src_2;
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
    return;
}

void t_swap_16bit(void *const src_1, void *const src_2)
{
    if (src_1 == src_2)
        return;
    uint16_t *a = (uint16_t *)src_1;
    uint16_t *b = (uint16_t *)src_2;
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
    return;
}

void t_swap_32bit(void *const src_1, void *const src_2)
{
    if (src_1 == src_2)
        return;
    uint32_t *a = (uint32_t *)src_1;
    uint32_t *b = (uint32_t *)src_2;
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
    return;
}

void t_swap_64bit(void *const src_1, void *const src_2)
{
    if (src_1 == src_2)
        return;
    uint64_t *a = (uint64_t *)src_1;
    uint64_t *b = (uint64_t *)src_2;
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
    return;
}

void t_swap_size_t(void *const src_1, void *const src_2)
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

/* At functions */
void *t_at_uint8_t(const void *const src, const size_t index)
{
    return ((uint8_t *)src) + index;
}

void *t_at_int8_t(const void *const src, const size_t index)
{
    return ((int8_t *)src) + index;
}

void *t_at_uint16_t(const void *const src, const size_t index)
{
    return ((uint16_t *)src) + index;
}

void *t_at_int16_t(const void *const src, const size_t index)
{
    return ((int16_t *)src) + index;
}

void *t_at_uint32_t(const void *const src, const size_t index)
{
    return ((uint32_t *)src) + index;
}

void *t_at_int32_t(const void *const src, const size_t index)
{
    return ((int32_t *)src) + index;
}

void *t_at_uint64_t(const void *const src, const size_t index)
{
    return ((uint64_t *)src) + index;
}

void *t_at_int64_t(const void *const src, const size_t index)
{
    return ((int64_t *)src) + index;
}

void *t_at_size_t(const void *const src, const size_t index)
{
    return ((size_t *)src) + index;
}

void *t_at_ssize_t(const void *const src, const size_t index)
{
    return ((ssize_t *)src) + index;
}

void *t_at_string_t(const void *const src, const size_t index)
{
    return ((string_t *)src) + index;
}

void *t_at_float(const void *const src, const size_t index)
{
    return ((float *)src) + index;
}

void *t_at_double(const void *const src, const size_t index)
{
    return ((double *)src) + index;
}

/* Cmp functions */
int t_cmp_uint8_t(const void *const src_1, const void *const src_2)
{
    uint8_t a = *(uint8_t *)src_1;
    uint8_t b = *(uint8_t *)src_2;
    return a > b ? 1 : (a < b ? -1 : 0);
}

int t_cmp_int8_t(const void *const src_1, const void *const src_2)
{
    int8_t a = *(int8_t *)src_1;
    int8_t b = *(int8_t *)src_2;
    return a > b ? 1 : (a < b ? -1 : 0);
}

int t_cmp_uint16_t(const void *const src_1, const void *const src_2)
{
    uint16_t a = *(uint16_t *)src_1;
    uint16_t b = *(uint16_t *)src_2;
    return a > b ? 1 : (a < b ? -1 : 0);
}

int t_cmp_int16_t(const void *const src_1, const void *const src_2)
{
    int16_t a = *(int16_t *)src_1;
    int16_t b = *(int16_t *)src_2;
    return a > b ? 1 : (a < b ? -1 : 0);
}

int t_cmp_uint32_t(const void *const src_1, const void *const src_2)
{
    uint32_t a = *(uint32_t *)src_1;
    uint32_t b = *(uint32_t *)src_2;
    return a > b ? 1 : (a < b ? -1 : 0);
}

int t_cmp_int32_t(const void *const src_1, const void *const src_2)
{
    int32_t a = *(int32_t *)src_1;
    int32_t b = *(int32_t *)src_2;
    return a > b ? 1 : (a < b ? -1 : 0);
}

int t_cmp_uint64_t(const void *const src_1, const void *const src_2)
{
    uint64_t a = *(uint64_t *)src_1;
    uint64_t b = *(uint64_t *)src_2;
    return a > b ? 1 : (a < b ? -1 : 0);
}

int t_cmp_int64_t(const void *const src_1, const void *const src_2)
{
    int64_t a = *(int64_t *)src_1;
    int64_t b = *(int64_t *)src_2;
    return a > b ? 1 : (a < b ? -1 : 0);
}

int t_cmp_size_t(const void *const src_1, const void *const src_2)
{
    size_t a = *(size_t *)src_1;
    size_t b = *(size_t *)src_2;
    return a > b ? 1 : (a < b ? -1 : 0);
}

int t_cmp_ssize_t(const void *const src_1, const void *const src_2)
{
    ssize_t a = *(ssize_t *)src_1;
    ssize_t b = *(ssize_t *)src_2;
    return a > b ? 1 : (a < b ? -1 : 0);
}

int t_cmp_string_t(const void *const src_1, const void *const src_2)
{
    return strcmp(get_string(*(const string_t *)src_1), get_string(*(const string_t *)src_2));
}

int t_cmp_float(const void *const src_1, const void *const src_2)
{
    float a = *(float *)src_1;
    float b = *(float *)src_2;
    float abs_diff = fabsf(a - b);
    return abs_diff < FLT_EPSILON ? 0 : (a > b ? 1 : -1);
}

int t_cmp_double(const void *const src_1, const void *const src_2)
{
    double a = *(double *)src_1;
    double b = *(double *)src_2;
    double abs_diff = fabs(a - b);
    return abs_diff < DBL_EPSILON ? 0 : (a > b ? 1 : -1);
}

/* Copy functions */
void *t_cpy_uint8_t(void *const dest, const void *const src)
{
    *(uint8_t *)dest = *(uint8_t *)src;
    return dest;
}

void *t_cpy_int8_t(void *const dest, const void *const src)
{
    *(int8_t *)dest = *(int8_t *)src;
    return dest;
}

void *t_cpy_uint16_t(void *const dest, const void *const src)
{
    *(uint16_t *)dest = *(uint16_t *)src;
    return dest;
}

void *t_cpy_int16_t(void *const dest, const void *const src)
{
    *(int16_t *)dest = *(int16_t *)src;
    return dest;
}

void *t_cpy_uint32_t(void *const dest, const void *const src)
{
    *(uint32_t *)dest = *(uint32_t *)src;
    return dest;
}

void *t_cpy_int32_t(void *const dest, const void *const src)
{
    *(int32_t *)dest = *(int32_t *)src;
    return dest;
}

void *t_cpy_uint64_t(void *const dest, const void *const src)
{
    *(uint64_t *)dest = *(uint64_t *)src;
    return dest;
}

void *t_cpy_int64_t(void *const dest, const void *const src)
{
    *(int64_t *)dest = *(int64_t *)src;
    return dest;
}

void *t_cpy_size_t(void *const dest, const void *const src)
{
    *(size_t *)dest = *(size_t *)src;
    return dest;
}

void *t_cpy_ssize_t(void *const dest, const void *const src)
{
    *(ssize_t *)dest = *(ssize_t *)src;
    return dest;
}

void *t_cpy_string_t(void *const dest, const void *const src)
{
    if (*(string_t *)dest != *(string_t *)src)
        *(string_t *)dest = copy_shared_ptr(*(string_t *)src);
    return dest;
}

void *t_cpy_float(void *const dest, const void *const src)
{
    *(float *)dest = *(float *)src;
    return dest;
}

void *t_cpy_double(void *const dest, const void *const src)
{
    *(double *)dest = *(double *)src;
    return dest;
}

/* Move functions */
void *t_move_uint8_t(void *const dest, const void *const src)
{
    *(uint8_t *)dest = *(uint8_t *)src;
    return dest;
}

void *t_move_int8_t(void *const dest, const void *const src)
{
    *(int8_t *)dest = *(int8_t *)src;
    return dest;
}

void *t_move_uint16_t(void *const dest, const void *const src)
{
    *(uint16_t *)dest = *(uint16_t *)src;
    return dest;
}

void *t_move_int16_t(void *const dest, const void *const src)
{
    *(int16_t *)dest = *(int16_t *)src;
    return dest;
}

void *t_move_uint32_t(void *const dest, const void *const src)
{
    *(uint32_t *)dest = *(uint32_t *)src;
    return dest;
}

void *t_move_int32_t(void *const dest, const void *const src)
{
    *(int32_t *)dest = *(int32_t *)src;
    return dest;
}

void *t_move_uint64_t(void *const dest, const void *const src)
{
    *(uint64_t *)dest = *(uint64_t *)src;
    return dest;
}

void *t_move_int64_t(void *const dest, const void *const src)
{
    *(int64_t *)dest = *(int64_t *)src;
    return dest;
}

void *t_move_size_t(void *const dest, const void *const src)
{
    *(size_t *)dest = *(size_t *)src;
    return dest;
}

void *t_move_ssize_t(void *const dest, const void *const src)
{
    *(ssize_t *)dest = *(ssize_t *)src;
    return dest;
}

void *t_move_string_t(void *const dest, const void *const src)
{
    *(string_t *)dest = *(string_t *)src;
    return dest;
}

void *t_move_float(void *const dest, const void *const src)
{
    *(float *)dest = *(float *)src;
    return dest;
}

void *t_move_double(void *const dest, const void *const src)
{
    *(double *)dest = *(double *)src;
    return dest;
}

/* Free functions */
void t_free_uint8_t(void *const src)
{
    uint8_t *val = (uint8_t *)src;
    *val = 0;
    return;
}

void t_free_int8_t(void *const src)
{
    int8_t *val = (int8_t *)src;
    *val = 0;
    return;
}

void t_free_uint16_t(void *const src)
{
    uint16_t *val = (uint16_t *)src;
    *val = 0;
    return;
}

void t_free_int16_t(void *const src)
{
    int16_t *val = (int16_t *)src;
    *val = 0;
    return;
}

void t_free_uint32_t(void *const src)
{
    uint32_t *val = (uint32_t *)src;
    *val = 0;
    return;
}

void t_free_int32_t(void *const src)
{
    int32_t *val = (int32_t *)src;
    *val = 0;
    return;
}

void t_free_uint64_t(void *const src)
{
    uint64_t *val = (uint64_t *)src;
    *val = 0;
    return;
}

void t_free_int64_t(void *const src)
{
    int64_t *val = (int64_t *)src;
    *val = 0;
    return;
}

void t_free_size_t(void *const src)
{
    size_t *val = (size_t *)src;
    *val = 0;
    return;
}

void t_free_ssize_t(void *const src)
{
    ssize_t *val = (ssize_t *)src;
    *val = 0;
    return;
}

void t_free_string_t(void *const src)
{
    string_t *val = (string_t *)src;
    free_string(*val);
    *val = NULL;
    return;
}

void t_free_float(void *const src)
{
    float *val = (float *)src;
    *val = 0.0f;
    return;
}

void t_free_double(void *const src)
{
    double *val = (double *)src;
    *val = 0.0;
    return;
}

/* Type functionality */
const type_func orig_f_uint8_t = {
    .t_size = sizeof(uint8_t),
    .t_at = t_at_uint8_t,
    .t_cmp = t_cmp_uint8_t,
    .t_cpy = t_cpy_uint8_t,
    .t_move = t_move_uint8_t,
    .t_free = t_free_uint8_t,
    .t_hash = t_hash_8bit,
    .t_swap = t_swap_8bit};

const type_func orig_f_int8_t = {
    .t_size = sizeof(int8_t),
    .t_at = t_at_int8_t,
    .t_cmp = t_cmp_int8_t,
    .t_cpy = t_cpy_int8_t,
    .t_move = t_move_int8_t,
    .t_free = t_free_int8_t,
    .t_hash = t_hash_8bit,
    .t_swap = t_swap_8bit};

const type_func orig_f_uint16_t = {
    .t_size = sizeof(uint16_t),
    .t_at = t_at_uint16_t,
    .t_cmp = t_cmp_uint16_t,
    .t_cpy = t_cpy_uint16_t,
    .t_move = t_move_uint16_t,
    .t_free = t_free_uint16_t,
    .t_hash = t_hash_16bit,
    .t_swap = t_swap_16bit};

const type_func orig_f_int16_t = {
    .t_size = sizeof(int16_t),
    .t_at = t_at_int16_t,
    .t_cmp = t_cmp_int16_t,
    .t_cpy = t_cpy_int16_t,
    .t_move = t_move_int16_t,
    .t_free = t_free_int16_t,
    .t_hash = t_hash_16bit,
    .t_swap = t_swap_16bit};

const type_func orig_f_uint32_t = {
    .t_size = sizeof(uint32_t),
    .t_at = t_at_uint32_t,
    .t_cmp = t_cmp_uint32_t,
    .t_cpy = t_cpy_uint32_t,
    .t_move = t_move_uint32_t,
    .t_free = t_free_uint32_t,
    .t_hash = t_hash_32bit,
    .t_swap = t_swap_32bit};
const type_func orig_f_int32_t = {
    .t_size = sizeof(int32_t),
    .t_at = t_at_int32_t,
    .t_cmp = t_cmp_int32_t,
    .t_cpy = t_cpy_int32_t,
    .t_move = t_move_int32_t,
    .t_free = t_free_int32_t,
    .t_hash = t_hash_32bit,
    .t_swap = t_swap_32bit};

const type_func orig_f_uint64_t = {
    .t_size = sizeof(uint64_t),
    .t_at = t_at_uint64_t,
    .t_cmp = t_cmp_uint64_t,
    .t_cpy = t_cpy_uint64_t,
    .t_move = t_move_uint64_t,
    .t_free = t_free_uint64_t,
    .t_hash = t_hash_64bit,
    .t_swap = t_swap_64bit};

const type_func orig_f_int64_t = {
    .t_size = sizeof(int64_t),
    .t_at = t_at_int64_t,
    .t_cmp = t_cmp_int64_t,
    .t_cpy = t_cpy_int64_t,
    .t_move = t_move_int64_t,
    .t_free = t_free_int64_t,
    .t_hash = t_hash_64bit,
    .t_swap = t_swap_64bit};

const type_func orig_f_size_t = {
    .t_size = sizeof(size_t),
    .t_at = t_at_size_t,
    .t_cmp = t_cmp_size_t,
    .t_cpy = t_cpy_size_t,
    .t_move = t_move_size_t,
    .t_free = t_free_size_t,
    .t_hash = t_hash_size_t,
    .t_swap = t_swap_size_t};

const type_func orig_f_ssize_t = {
    .t_size = sizeof(ssize_t),
    .t_at = t_at_ssize_t,
    .t_cmp = t_cmp_ssize_t,
    .t_cpy = t_cpy_ssize_t,
    .t_move = t_move_ssize_t,
    .t_free = t_free_ssize_t,
    .t_hash = t_hash_size_t,
    .t_swap = t_swap_size_t};

const type_func orig_f_string_t = {
    .t_size = sizeof(string_t),
    .t_at = t_at_string_t,
    .t_cmp = t_cmp_string_t,
    .t_cpy = t_cpy_string_t,
    .t_move = t_move_string_t,
    .t_free = t_free_string_t,
    .t_hash = t_hash_string_t,
    .t_swap = t_swap_size_t};

const type_func orig_f_float = {
    .t_size = sizeof(float),
    .t_at = t_at_float,
    .t_cmp = t_cmp_float,
    .t_cpy = t_cpy_float,
    .t_move = t_move_float,
    .t_free = t_free_float,
    .t_hash = t_hash_32bit,
    .t_swap = t_swap_32bit};

const type_func orig_f_double = {
    .t_size = sizeof(double),
    .t_at = t_at_double,
    .t_cmp = t_cmp_double,
    .t_cpy = t_cpy_double,
    .t_move = t_move_double,
    .t_free = t_free_double,
    .t_hash = t_hash_64bit,
    .t_swap = t_swap_64bit};

/* Pointers */
const type_func *f_uint8_t = &orig_f_uint8_t;
const type_func *f_int8_t = &orig_f_int8_t;
const type_func *f_uint16_t = &orig_f_uint16_t;
const type_func *f_int16_t = &orig_f_int16_t;
const type_func *f_uint32_t = &orig_f_uint32_t;
const type_func *f_int32_t = &orig_f_int32_t;
const type_func *f_uint64_t = &orig_f_uint64_t;
const type_func *f_int64_t = &orig_f_int64_t;
const type_func *f_size_t = &orig_f_size_t;
const type_func *f_ssize_t = &orig_f_ssize_t;
const type_func *f_string_t = &orig_f_string_t;
const type_func *f_float = &orig_f_float;
const type_func *f_double = &orig_f_double;
