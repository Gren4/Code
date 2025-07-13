#include "string_type.h"
#include <string.h>

string_t create_string(const char *const str)
{
    string_t new_string = malloc_shared_ptr(strlen(str) + 1, sizeof(char));
    strcpy(data_shared_ptr(new_string), str);
    return new_string;
}

char * get_string(string_t const str)
{
    return data_shared_ptr(str);
}

void free_string(string_t const str)
{
    free_shared_ptr(str);
    return;
}
