#pragma once

#include <stdint.h>

#define nullptr 0

size_t next_power_of_2(size_t num);
size_t djb2(const uint8_t *key, size_t key_len);
size_t djb2_str(const uint8_t *key, size_t key_len);
size_t simple_hash(const uint8_t *key, size_t key_len);
size_t sdbm(const uint8_t *key, size_t key_len);
size_t sdbm_str(const uint8_t *key, size_t key_len);
