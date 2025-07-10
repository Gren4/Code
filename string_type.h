#pragma once

typedef struct string
{
    char *data;
} string;

string create_string(const char *const str);
void *cpy_string(void *const dest, const void *const src);
void free_string(void *const str);