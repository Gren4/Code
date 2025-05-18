#pragma once

#include <stdint.h>

#define nullptr 0

size_t next_power_of_2(size_t num);
size_t calculate_padding(size_t current_addres, size_t data_size);
