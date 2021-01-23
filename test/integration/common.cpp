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



bool file_exists_in_path(const char *tmppath, const char* relpath)
{
    char fullpath[MAX_PATH_LEN];
    fullpath[0] = '\0';
    snprintf((char*)fullpath, sizeof(fullpath), "%s/%s", tmppath, relpath);
    std::error_code error_code;
    return fs::exists(fullpath, error_code);
}

void setup(char *tmppath, size_t tmppath_size)
{
    // Creates a temp-dir and returns the path to it
    std::error_code error_code;
    fs::path tmp_dir = fs::temp_directory_path(error_code);

    // Assign semi-unique subfolder name to allow parallell runs. Att! Not particularily robust.
    bool dir_created = false;
    for(int i=0; i<100; i++) {
        snprintf(tmppath, tmppath_size, "%s%s_%d", tmp_dir.c_str(), "prosit_inttest", i);
        printf("Attempting to create tmp-folder: %s\n", tmppath);
        if(fs::create_directories(tmppath, error_code)) {
            printf("Succeeded in creating: %s\n", tmppath);
            dir_created = true;
            break;
        }
    }
    assert(dir_created);
}

void teardown(char *tmppath)
{
    // Deletes the folder adressed by tmppath and all of its contents
    assert(fs::remove_all(tmppath) > 0);
}

// out_tmppath must be cleaned, and then *out_tmppath deleted
App_Status_Code basic_app_main_run_no_teardown(const char *manifest, char** out_tmppath)
{
    char* tmppath = new char[MAX_PATH_LEN];
    fs::path initial_path = fs::current_path();
    fs::path manifest_path = fs::canonical(manifest);
    fs::path testfiles_path = fs::canonical("../test/integration/testfiles");
    setenv("PROSIT_ITEST_TESTFILES", (char *)testfiles_path.u8string().c_str(), 1);

    // TODO: Check resulting contents instead of just the return code.

    char *argv[] = {
        (char *)"prosit",
        (char *)"update",
        (char *)NULL,
        (char *)"--verbose"};

    setup(tmppath, MAX_PATH_LEN);
    defer(fs::current_path(initial_path));

    char manifest_arg[1024];
    snprintf(manifest_arg, sizeof(manifest_arg), "--manifest=%s", manifest_path.u8string().c_str());
    fs::current_path(tmppath);
    argv[2] = manifest_arg;

    *out_tmppath = tmppath;

    return app_main(4, argv);
}


App_Status_Code basic_app_main_run(const char *manifest)
{
    char tmppath[MAX_PATH_LEN];
    fs::path initial_path = fs::current_path();
    fs::path manifest_path = fs::canonical(manifest);
    fs::path testfiles_path = fs::canonical("../test/integration/testfiles");
    setenv("PROSIT_ITEST_TESTFILES", (char *)testfiles_path.u8string().c_str(), 1);

    // TODO: Check resulting contents instead of just the return code.

    char *argv[] = {
        (char *)"prosit",
        (char *)"update",
        (char *)NULL};

    setup(tmppath, MAX_PATH_LEN);
    defer(teardown(tmppath));
    defer(fs::current_path(initial_path));

    char manifest_arg[1024];
    snprintf(manifest_arg, sizeof(manifest_arg), "--manifest=%s", manifest_path.u8string().c_str());
    fs::current_path(tmppath);
    argv[2] = manifest_arg;

    return app_main(3, argv);
}
