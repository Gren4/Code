#pragma once

#include <stdint.h>

#define nullptr 0

size_t next_power_of_2(size_t num);
size_t djb2(uint8_t *key, size_t key_len);
size_t sdbm(uint8_t *key, size_t key_len);
