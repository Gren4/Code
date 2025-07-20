#pragma once

#include "bit_types.h"

size_t next_power_of_2(size_t num);
size_t calculate_padding(size_t current_addres, size_t data_size);
size_t hash_8bit(uint8_t x);
size_t hash_16bit(uint16_t x);
size_t hash_32bit(uint32_t x);
size_t hash_64bit(uint64_t x);
size_t hash_size_t(size_t x);
size_t hash_string_t(const char *key);