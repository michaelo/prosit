#pragma once

#include <cstdio>
#include <cstring>
#include <cassert>

#include "defer.imp.h"

inline char *file_to_buf(const char *path, size_t *size_out = NULL)
{
    FILE *fp = fopen(path, "r");
    defer(fclose(fp));

    if (fp == NULL)
    {
        return NULL;
    }

    fseek(fp, 0L, SEEK_END);
    long raw_size = ftell(fp);
    rewind(fp);

    char *filebuf = new char[raw_size];
    fread(filebuf, raw_size, 1, fp);

    if (size_out != NULL)
        *size_out = raw_size;
    return filebuf; // must free
}

template <typename Functor>
void buf_pr_token(char *buf, const char *sep, Functor callback)
{
    char *save;
    char *token = strtok_r(buf, sep, &save);
    while (token != NULL)
    {
        // TBD: Don't pass empty chunks?
        callback(token);
        token = strtok_r(NULL, sep, &save);
    }
}
