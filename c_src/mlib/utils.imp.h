#pragma once

#include <cstdio>
#include <cstring>
#include <cassert>

#include "defer.imp.h"

#ifdef _WIN32

inline char* strtok_r(char* str, const char* sep, char** save)
{
    return strtok_s(str, sep, save);
}
#endif

inline char *file_to_buf(const char *path, size_t *size_out = nullptr)
{
    // TODO: Consider rb vs r and rather handle size by reading byte-by-byte.
    FILE *fp = fopen(path, "rb");
    if (fp == nullptr)
    {
        return nullptr;
    }
    defer(fclose(fp));

    fseek(fp, 0L, SEEK_END);
    long raw_size = ftell(fp);
    rewind(fp);

    char *filebuf = new char[raw_size+1];
    fread(filebuf, raw_size, 1, fp);
    filebuf[raw_size] = '\0';

    if (size_out != nullptr)
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
