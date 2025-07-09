#include "string_type.h"
#include <stdlib.h>
#include <string.h>

string create_string(const char *str)
{
    char *ptr = calloc(strlen(str) + 1, sizeof(char));
    string new_string = {
        .data = strcpy(ptr, str)};
    return new_string;
}

void *cpy_string(void *dest, const void *src)
{
    string *str_1 = (string *)dest;
    const string *str_2 = (const string *)src;
    str_1->data = str_1->data == NULL ? calloc(strlen(str_2->data) + 1, sizeof(char)) : realloc(str_1->data, (strlen(str_2->data) + 1) * sizeof(char));
    if (str_1->data != NULL)
        str_1->data = strcpy(str_1->data, str_2->data);
    return dest;
}

void free_string(void *src)
{
    string *str = (string *)src;
    if (str->data != NULL)
        free(str->data);
    str->data = NULL;
    return;
}
