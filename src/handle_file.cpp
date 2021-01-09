#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <cassert>

#include "app.h"

#include "mlib/defer.imp.h"

namespace fs = std::filesystem;

App_Status_Code handle_file(Context *c, Manifest_Entry *e)
{
    c->debug("About to copy %s to %s\n", e->src, e->dst, e->line_in_manifest);
    // TODO: Do modified-check before copy? Unless e.g. -f / !

    fs::path src_canonical = fs::weakly_canonical(e->src);
    size_t dst_len = strlen(e->dst);

    if(e->dst[strlen(e->dst)-1] == '/') {
        // TODO: Ensure we're within buffer size
        assert(sizeof(e->dst) > dst_len+src_canonical.filename().string().length());
        strcpy(&e->dst[strlen(e->dst)], (const char*)src_canonical.filename().c_str());
    }

    if(!fs::exists(e->src)) {
        c->error("No such file: %s\n", e->src);
        return App_Status_Code::Error;
    }
    
    if(!fs::exists(fs::path(e->dst).parent_path()) && !fs::create_directories(fs::path(e->dst).parent_path()))
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
