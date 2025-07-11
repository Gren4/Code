#pragma once

#include <stdint.h>
#include <assert.h>

#define byte(i_1, i_2, i_3, i_4, i_5, i_6, i_7, i_8) \
    uint8_t bit_##i_1 : 1;                           \
    uint8_t bit_##i_2 : 1;                           \
    uint8_t bit_##i_3 : 1;                           \
    uint8_t bit_##i_4 : 1;                           \
    uint8_t bit_##i_5 : 1;                           \
    uint8_t bit_##i_6 : 1;                           \
    uint8_t bit_##i_7 : 1;                           \
    uint8_t bit_##i_8 : 1

#if (defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)) || defined(__LITTLE_ENDIAN__)

#define bit8_t \
    byte(1, 2, 3, 4, 5, 6, 7, 8)

#define bit16_t                   \
    byte(1, 2, 3, 4, 5, 6, 7, 8); \
    byte(9, 10, 11, 12, 13, 14, 15, 16)

#define bit32_t                           \
    byte(1, 2, 3, 4, 5, 6, 7, 8);         \
    byte(9, 10, 11, 12, 13, 14, 15, 16);  \
    byte(17, 18, 19, 20, 21, 22, 23, 24); \
    byte(25, 26, 27, 28, 29, 30, 31, 32)

#define bit64_t                           \
    byte(1, 2, 3, 4, 5, 6, 7, 8);         \
    byte(9, 10, 11, 12, 13, 14, 15, 16);  \
    byte(17, 18, 19, 20, 21, 22, 23, 24); \
    byte(25, 26, 27, 28, 29, 30, 31, 32); \
    byte(33, 34, 35, 36, 37, 38, 39, 40); \
    byte(41, 42, 43, 44, 45, 46, 47, 48); \
    byte(49, 50, 51, 52, 53, 54, 55, 56); \
    byte(57, 58, 59, 60, 61, 62, 63, 64)

#elif (defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN)) || defined(__BIG_ENDIAN__)

#define bit8_t \
    byte(1, 2, 3, 4, 5, 6, 7, 8)

#define bit16_t                          \
    byte(9, 10, 11, 12, 13, 14, 15, 16); \
    byte(1, 2, 3, 4, 5, 6, 7, 8)

#define bit32_t                           \
    byte(25, 26, 27, 28, 29, 30, 31, 32); \
    byte(17, 18, 19, 20, 21, 22, 23, 24); \
    byte(9, 10, 11, 12, 13, 14, 15, 16);  \
    byte(1, 2, 3, 4, 5, 6, 7, 8)

#define bit64_t                           \
    byte(57, 58, 59, 60, 61, 62, 63, 64); \
    byte(49, 50, 51, 52, 53, 54, 55, 56); \
    byte(41, 42, 43, 44, 45, 46, 47, 48); \
    byte(33, 34, 35, 36, 37, 38, 39, 40); \
    byte(25, 26, 27, 28, 29, 30, 31, 32); \
    byte(17, 18, 19, 20, 21, 22, 23, 24); \
    byte(9, 10, 11, 12, 13, 14, 15, 16);  \
    byte(1, 2, 3, 4, 5, 6, 7, 8)

#else

#error Nonstandart byte order

#endif

#define assert_error_string(x) #x
#define bit_type_def(type_name, bits_count)                                                                               \
    static_assert((sizeof(type_name) << 3) == bits_count, assert_error_string(Incorrect bits count for type type_name)); \
    typedef union b_##type_name                                                                                           \
    {                                                                                                                    \
        type_name value;                                                                                                 \
        struct                                                                                                           \
        {                                                                                                                \
            bit##bits_count##_t;                                                                                         \
        };                                                                                                               \
    } b_##type_name

