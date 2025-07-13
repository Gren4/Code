#pragma once
#include "shared_ptr.h"

typedef shared_ptr string_t;

string_t create_string(const char *const str);
char * get_string(string_t const str);
void free_string(string_t const str);