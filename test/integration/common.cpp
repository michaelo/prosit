#include "itest.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <filesystem>

#include "app.h"
#include "mlib/defer.imp.h"
#include "platform.h"

namespace fs = std::filesystem;

// TODO: Replace tmpnam() with appropriate non-obsolete substitutes

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

App_Status_Code basic_app_main_run(const char* manifest) {
    char tmppath[L_tmpnam];
    fs::path initial_path = fs::current_path();
    fs::path manifest_path = fs::canonical(manifest);
    fs::path testfiles_path = fs::canonical("../test/integration/testfiles");
    setenv("PROSIT_ITEST_TESTFILES", (char*)testfiles_path.u8string().c_str(), 1);

    // TODO: Check resulting contents instead of just the return code.

    char *argv[] = {
        (char *)"prosit",
        (char *)"update",
        (char *)NULL};

    setup(tmppath);
    defer(teardown(tmppath));
    defer(fs::current_path(initial_path));

    char manifest_arg[1024];
    snprintf(manifest_arg, sizeof(manifest_arg), "--manifest=%s", manifest_path.u8string().c_str());
    fs::current_path(tmppath);
    argv[2] = manifest_arg;

    return app_main(3, argv);
}