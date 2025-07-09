#pragma once

typedef struct string
{
    char *data;
} string;

string create_string(const char *str);
void *cpy_string(void *dest, const void *src);
void free_string(void *str);