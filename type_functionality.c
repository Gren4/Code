#include "type_functionality.h"
#include "string_type.h"
#include <stdint.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <string.h>

/* hash funcs */
size_t hash_8bit(const void *src)
{
    uint8_t x = *(uint8_t *)src;
    x = ((x >> 3) ^ x) * 0x1D;
    x = ((x >> 5) ^ x) * 0x35;
    return x;
}
size_t hash_16bit(const void *src)
{
    uint16_t x = *(uint16_t *)src;
    x = ((x >> 8) ^ x) * 0x88CC;
    x = ((x >> 7) ^ x) * 0x119D;
    return x;
}
size_t hash_32bit(const void *src)
{
    uint32_t x = *(uint32_t *)src;
    x = ((x >> 16) ^ x) * 0x45D9F3B;
    x = ((x >> 16) ^ x) * 0x45D9F3B;
    return x ^ (x >> 16);
}
size_t hash_64bit(const void *src)
{
    uint64_t x = *(uint64_t *)src;
    x = ((x >> 30) ^ x) * 0xBF58476D1CE4E5B9;
    x = ((x >> 27) ^ x) * 0x94D049BB133111EB;
    return x ^ (x >> 31);
}
size_t hash_string(const void *src)
{
    /* sdbm */
    const uint8_t *key = (uint8_t *)src;
    size_t hash = 0;
    size_t c;
    while ((c = *key++))
    {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}

/* uint8_t */
void *at_uint8_t(const void *src, const size_t index)
{
    return ((uint8_t *)src) + index;
}
int cmp_uint8_t(const void *src_1, const void *src_2)
{
    uint8_t a = *(uint8_t *)src_1;
    uint8_t b = *(uint8_t *)src_2;
    return a > b ? 1 : (a < b ? -1 : 0);
}
void *cpy_uint8_t(void *dest, const void *src)
{
    *(uint8_t *)dest = *(uint8_t *)src;
    return dest;
}
const type_func orig_f_uint8_t = {
    .t_size = sizeof(uint8_t),
    .t_at = at_uint8_t,
    .t_cmp = cmp_uint8_t,
    .t_cpy = cpy_uint8_t,
    .t_move = cpy_uint8_t,
    .t_free = NULL,
    .t_hash = hash_8bit};

/* int8_t */
void *at_int8_t(const void *src, const size_t index)
{
    return ((int8_t *)src) + index;
}
int cmp_int8_t(const void *src_1, const void *src_2)
{
    int8_t a = *(int8_t *)src_1;
    int8_t b = *(int8_t *)src_2;
    return a > b ? 1 : (a < b ? -1 : 0);
}
void *cpy_int8_t(void *dest, const void *src)
{
    *(int8_t *)dest = *(int8_t *)src;
    return dest;
}
const type_func orig_f_int8_t = {
    .t_size = sizeof(int8_t),
    .t_at = at_int8_t,
    .t_cmp = cmp_int8_t,
    .t_cpy = cpy_int8_t,
    .t_move = cpy_int8_t,
    .t_free = NULL,
    .t_hash = hash_8bit};

/* uint16_t */
void *at_uint16_t(const void *src, const size_t index)
{
    return ((uint16_t *)src) + index;
}
int cmp_uint16_t(const void *src_1, const void *src_2)
{
    uint16_t a = *(uint16_t *)src_1;
    uint16_t b = *(uint16_t *)src_2;
    return a > b ? 1 : (a < b ? -1 : 0);
}
void *cpy_uint16_t(void *dest, const void *src)
{
    *(uint16_t *)dest = *(uint16_t *)src;
    return dest;
}
const type_func orig_f_uint16_t = {
    .t_size = sizeof(uint16_t),
    .t_at = at_uint16_t,
    .t_cmp = cmp_uint16_t,
    .t_cpy = cpy_uint16_t,
    .t_move = cpy_uint16_t,
    .t_free = NULL,
    .t_hash = hash_16bit};

/* int16_t */
void *at_int16_t(const void *src, const size_t index)
{
    return ((int16_t *)src) + index;
}
int cmp_int16_t(const void *src_1, const void *src_2)
{
    int16_t a = *(int16_t *)src_1;
    int16_t b = *(int16_t *)src_2;
    return a > b ? 1 : (a < b ? -1 : 0);
}
void *cpy_int16_t(void *dest, const void *src)
{
    *(int16_t *)dest = *(int16_t *)src;
    return dest;
}
const type_func orig_f_int16_t = {
    .t_size = sizeof(int16_t),
    .t_at = at_int16_t,
    .t_cmp = cmp_int16_t,
    .t_cpy = cpy_int16_t,
    .t_move = cpy_int16_t,
    .t_free = NULL,
    .t_hash = hash_16bit};

/* uint32_t */
void *at_uint32_t(const void *src, const size_t index)
{
    return ((uint32_t *)src) + index;
}
int cmp_uint32_t(const void *src_1, const void *src_2)
{
    uint32_t a = *(uint32_t *)src_1;
    uint32_t b = *(uint32_t *)src_2;
    return a > b ? 1 : (a < b ? -1 : 0);
}
void *cpy_uint32_t(void *dest, const void *src)
{
    *(uint32_t *)dest = *(uint32_t *)src;
    return dest;
}
const type_func orig_f_uint32_t = {
    .t_size = sizeof(uint32_t),
    .t_at = at_uint32_t,
    .t_cmp = cmp_uint32_t,
    .t_cpy = cpy_uint32_t,
    .t_move = cpy_uint32_t,
    .t_free = NULL,
    .t_hash = hash_32bit};

/* int32_t */
void *at_int32_t(const void *src, const size_t index)
{
    return ((int32_t *)src) + index;
}
int cmp_int32_t(const void *src_1, const void *src_2)
{
    int32_t a = *(int32_t *)src_1;
    int32_t b = *(int32_t *)src_2;
    return a > b ? 1 : (a < b ? -1 : 0);
}
void *cpy_int32_t(void *dest, const void *src)
{
    *(int32_t *)dest = *(int32_t *)src;
    return dest;
}
const type_func orig_f_int32_t = {
    .t_size = sizeof(int32_t),
    .t_at = at_int32_t,
    .t_cmp = cmp_int32_t,
    .t_cpy = cpy_int32_t,
    .t_move = cpy_int32_t,
    .t_free = NULL,
    .t_hash = hash_32bit};

/* uint64_t */
void *at_uint64_t(const void *src, const size_t index)
{
    return ((uint64_t *)src) + index;
}
int cmp_uint64_t(const void *src_1, const void *src_2)
{
    uint64_t a = *(uint64_t *)src_1;
    uint64_t b = *(uint64_t *)src_2;
    return a > b ? 1 : (a < b ? -1 : 0);
}
void *cpy_uint64_t(void *dest, const void *src)
{
    *(uint64_t *)dest = *(uint64_t *)src;
    return dest;
}
const type_func orig_f_uint64_t = {
    .t_size = sizeof(uint64_t),
    .t_at = at_uint64_t,
    .t_cmp = cmp_uint64_t,
    .t_cpy = cpy_uint64_t,
    .t_move = cpy_uint64_t,
    .t_free = NULL,
    .t_hash = hash_64bit};

/* int64_t */
void *at_int64_t(const void *src, const size_t index)
{
    return ((int64_t *)src) + index;
}
int cmp_int64_t(const void *src_1, const void *src_2)
{
    int64_t a = *(int64_t *)src_1;
    int64_t b = *(int64_t *)src_2;
    return a > b ? 1 : (a < b ? -1 : 0);
}
void *cpy_int64_t(void *dest, const void *src)
{
    *(int64_t *)dest = *(int64_t *)src;
    return dest;
}
const type_func orig_f_int64_t = {
    .t_size = sizeof(int64_t),
    .t_at = at_int64_t,
    .t_cmp = cmp_int64_t,
    .t_cpy = cpy_int64_t,
    .t_move = cpy_int64_t,
    .t_free = NULL,
    .t_hash = hash_64bit};

/* size_t */
void *at_size_t(const void *src, const size_t index)
{
    return ((size_t *)src) + index;
}
int cmp_size_t(const void *src_1, const void *src_2)
{
    size_t a = *(size_t *)src_1;
    size_t b = *(size_t *)src_2;
    return a > b ? 1 : (a < b ? -1 : 0);
}
void *cpy_size_t(void *dest, const void *src)
{
    *(size_t *)dest = *(size_t *)src;
    return dest;
}
const type_func orig_f_size_t = {
    .t_size = sizeof(size_t),
    .t_at = at_size_t,
    .t_cmp = cmp_size_t,
    .t_cpy = cpy_size_t,
    .t_move = cpy_size_t,
    .t_free = NULL,
    .t_hash = sizeof(size_t) == sizeof(int64_t) ? hash_64bit : hash_32bit};

/* ssize_t */
void *at_ssize_t(const void *src, const size_t index)
{
    return ((ssize_t *)src) + index;
}
int cmp_ssize_t(const void *src_1, const void *src_2)
{
    ssize_t a = *(ssize_t *)src_1;
    ssize_t b = *(ssize_t *)src_2;
    return a > b ? 1 : (a < b ? -1 : 0);
}
void *cpy_ssize_t(void *dest, const void *src)
{
    *(ssize_t *)dest = *(ssize_t *)src;
    return dest;
}
const type_func orig_f_ssize_t = {
    .t_size = sizeof(ssize_t),
    .t_at = at_ssize_t,
    .t_cmp = cmp_ssize_t,
    .t_cpy = cpy_ssize_t,
    .t_move = cpy_ssize_t,
    .t_free = NULL,
    .t_hash = sizeof(size_t) == sizeof(int64_t) ? hash_64bit : hash_32bit};

/* string */
void *at_string(const void *src, const size_t index)
{
    return ((string *)src) + index;
}
int cmp_string(const void *src_1, const void *src_2)
{
    const string *str_1 = (const string *)src_1;
    const string *str_2 = (const string *)src_2;
    return strcmp(str_1->data, str_2->data);
}
void *move_string(void *dest, const void *src)
{
    return memcpy((string *)dest, (const string *)src, sizeof(string));
}
const type_func orig_f_string = {
    .t_size = sizeof(string),
    .t_at = at_string,
    .t_cmp = cmp_string,
    .t_cpy = cpy_string,
    .t_move = move_string,
    .t_free = free_string,
    .t_hash = hash_string};

/* float */
void *at_float(const void *src, const size_t index)
{
    return ((float *)src) + index;
}
int cmp_float(const void *src_1, const void *src_2)
{
    float a = *(float *)src_1;
    float b = *(float *)src_2;
    float abs_diff = fabsf(a - b);
    return abs_diff < FLT_EPSILON ? 0 : (a > b ? 1 : -1);
}
void *cpy_float(void *dest, const void *src)
{
    *(float *)dest = *(float *)src;
    return dest;
}
const type_func orig_f_float = {
    .t_size = sizeof(float),
    .t_at = at_float,
    .t_cmp = cmp_float,
    .t_cpy = cpy_float,
    .t_move = cpy_float,
    .t_free = NULL,
    .t_hash = hash_32bit};

/* double */
void *at_double(const void *src, const size_t index)
{
    return ((double *)src) + index;
}
int cmp_double(const void *src_1, const void *src_2)
{
    double a = *(double *)src_1;
    double b = *(double *)src_2;
    double abs_diff = fabs(a - b);
    return abs_diff < DBL_EPSILON ? 0 : (a > b ? 1 : -1);
}
void *cpy_double(void *dest, const void *src)
{
    *(double *)dest = *(double *)src;
    return dest;
}
const type_func orig_f_double = {
    .t_size = sizeof(double),
    .t_at = at_double,
    .t_cmp = cmp_double,
    .t_cpy = cpy_double,
    .t_move = cpy_double,
    .t_free = NULL,
    .t_hash = hash_64bit};

/* pointers */
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
const type_func *f_string = &orig_f_string;
const type_func *f_float = &orig_f_float;
const type_func *f_double = &orig_f_double;