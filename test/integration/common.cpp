#include "itest.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

void setup(char *tmppath)
{
    // Creates a temp-dir and returns the path to it
    char tmp_name[L_tmpnam];
    tmpnam(tmp_name);
    assert(fs::create_directories(tmp_name));
    strcpy(tmppath, tmp_name);
}

void teardown(char *tmppath)
{
    // Deletes the folder adressed by tmppath and all of its contents
    assert(fs::remove_all(tmppath) > 0);
}


