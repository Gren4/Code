#pragma once

#include "bit_types_macros.h"

bit_type_def(uint8_t, 8);
bit_type_def(int8_t, 8);
bit_type_def(uint16_t, 16);
bit_type_def(int16_t, 16);
bit_type_def(uint32_t, 32);
bit_type_def(int32_t, 32);
bit_type_def(uint64_t, 64);
bit_type_def(int64_t, 64);
#if SIZE_MAX == UINT32_MAX
bit_type_def(size_t, 32);
bit_type_def(ssize_t, 32);
#elif SIZE_MAX == UINT64_MAX
bit_type_def(size_t, 64);
bit_type_def(ssize_t, 64);
#endif // SIZE_MAX
bit_type_def(float, 32);
bit_type_def(double, 64);
