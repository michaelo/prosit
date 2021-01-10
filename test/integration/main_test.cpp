#include "app.h"
#include "test.h"

#include "mlib/defer.imp.h"

#include <cassert>
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

TEST(MainTest, do_nothing_with_empty_manifest)
{
}

TEST(MainTest, test_https)
{
    char tmppath[L_tmpnam];
    fs::path initial_path = fs::current_path();

    {
        setup(tmppath);
        defer(teardown(tmppath));
        defer(fs::current_path(initial_path));

        fs::path manifest_path = fs::canonical("../test/integration/testfiles/empty.manifest");

        char manifest_arg[1024];
        snprintf(manifest_arg, sizeof(manifest_arg), "--manifest=%s", manifest_path.c_str());
        fs::current_path(tmppath);
        assert(app_main(3, (char *[]){
                               (char *)"prosit",
                               (char *)"update",
                               (char *)manifest_arg}) == 0);
    }

    // Get file from plain http
    // TODO

    // Get unprotected file from https
    {
        setup(tmppath);
        defer(teardown(tmppath));
        defer(fs::current_path(initial_path));

        fs::path manifest_path = fs::canonical("../test/integration/testfiles/https.manifest");

        char manifest_arg[1024];
        snprintf(manifest_arg, sizeof(manifest_arg), "--manifest=%s", manifest_path.c_str());
        fs::current_path(tmppath);
        assert(app_main(3, (char *[]){
                               (char *)"prosit",
                               (char *)"update",
                               (char *)manifest_arg}) == App_Status_Code::OK);
    }

    // Get basic auth protected http file
    // TODO
    
    // Get basic auth protected https file
    {
        setup(tmppath);
        defer(teardown(tmppath));
        defer(fs::current_path(initial_path));

        fs::path manifest_path = fs::canonical("../test/integration/testfiles/https_basic_auth.manifest");

        char manifest_arg[1024];
        snprintf(manifest_arg, sizeof(manifest_arg), "--manifest=%s", manifest_path.c_str());
        fs::current_path(tmppath);
        assert(app_main(3, (char *[]){
                               (char *)"prosit",
                               (char *)"update",
                               (char *)manifest_arg}) == App_Status_Code::OK);
    }

    // Fails if no auth-details provided for auth protected file
    {
        setup(tmppath);
        defer(teardown(tmppath));
        defer(fs::current_path(initial_path));

        fs::path manifest_path = fs::canonical("../test/integration/testfiles/https_basic_auth_missing_login.manifest");

        char manifest_arg[1024];
        snprintf(manifest_arg, sizeof(manifest_arg), "--manifest=%s", manifest_path.c_str());
        fs::current_path(tmppath);
        assert(app_main(3, (char *[]){
                               (char *)"prosit",
                               (char *)"update",
                               (char *)manifest_arg}) != App_Status_Code::OK);
    }

    // Fails if incorrect auth-details provided for auth protected file
    {
        setup(tmppath);
        defer(teardown(tmppath));
        defer(fs::current_path(initial_path));

        fs::path manifest_path = fs::canonical("../test/integration/testfiles/https_basic_auth_incorrect_login.manifest");

        char manifest_arg[1024];
        snprintf(manifest_arg, sizeof(manifest_arg), "--manifest=%s", manifest_path.c_str());
        fs::current_path(tmppath);
        assert(app_main(3, (char *[]){
                               (char *)"prosit",
                               (char *)"update",
                               (char *)manifest_arg}) != App_Status_Code::OK);
    }
}

TEST(MainTest, test_file)
{
    // Copy local file to specific dest name
    // Copy local file to folder (keeps original filename)
    // Fails if src does not exist
    // ASSERT_TRUE(false);
}

TEST(MainTest, test_git)
{
    // Clone git-repo from local path
    // Clone git-repo from remote path
    // Update existing clone
    // Fails if cloning non-existent repo
    // ASSERT_TRUE(false);
}

TEST(MainTest, test_manifest_load)
{
    // Tests to verify robustness and integrity of manifest parser
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}