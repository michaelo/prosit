#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <cassert>
#include <iostream>

#include "app.h"

#include "mlib/defer.imp.h"

namespace fs = std::filesystem;

App_Status_Code handle_file(Context *c, Manifest_Entry *e)
{
    c->debug("About to copy %s to %s\n", e->src, e->dst, e->line_in_manifest);
    // TODO: Do modified-check before copy? Unless e.g. -f / !

    fs::path src_canonical = fs::canonical(e->src);
    size_t dst_len = strlen(e->dst);
    // std::error_code error_code; // TODO: get rid of std-exceptions.

    if(e->dst[strlen(e->dst)-1] == '/') {
        assert(sizeof(e->dst) > dst_len+src_canonical.filename().string().length());
        strcpy(&e->dst[strlen(e->dst)], (const char*)src_canonical.filename().u8string().c_str());
    }

    if(!fs::exists(e->src)) {
        c->error("No such file: %s\n", e->src);
        return App_Status_Code::Error;
    }

    fs::path dst_folder = fs::weakly_canonical(e->dst).parent_path();

    if(!fs::exists(dst_folder) && !fs::create_directories(dst_folder))
    {
        c->error("Could not copy file: %s, could not create directory\n", e->src);
        return App_Status_Code::Error;
    }

    if(!fs::copy_file(e->src, e->dst, fs::copy_options::overwrite_existing)) {
        c->error("Could not copy file: %s\n", e->src);
        return App_Status_Code::Error;
    }

    c->debug("Copy of %s to %s OK\n", e->src, e->dst);

    return App_Status_Code::OK;
}
