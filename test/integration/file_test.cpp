#include "app.h"
#include "test.h"
#include "itest.h"

#include "mlib/defer.imp.h"

#include <cassert>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

TEST(FileTst, test_file)
{
    char tmppath[L_tmpnam];
    fs::path initial_path = fs::current_path();
    fs::path testfiles_path = fs::canonical("../test/integration/testfiles");
    setenv("PROSIT_ITEST_TESTFILES", testfiles_path.c_str(), 1);

    char *argv[] = {
        (char *)"prosit",
        (char *)"update",
        (char *)NULL};

    // Copy local file to specific dest name
    {
        setup(tmppath);
        defer(teardown(tmppath));
        defer(fs::current_path(initial_path));

        fs::path manifest_path = fs::canonical("../test/integration/testfiles/file.manifest");

        char manifest_arg[1024];
        snprintf(manifest_arg, sizeof(manifest_arg), "--manifest=%s", manifest_path.c_str());
        fs::current_path(tmppath);
        argv[2] = manifest_arg;
        ASSERT_EQ(app_main(3, argv), App_Status_Code::OK);
    }

    
    // Copy local file to folder (keeps original filename)
    {
        setup(tmppath);
        defer(teardown(tmppath));
        defer(fs::current_path(initial_path));

        fs::path manifest_path = fs::canonical("../test/integration/testfiles/file_no_dest_name.manifest");

        char manifest_arg[1024];
        snprintf(manifest_arg, sizeof(manifest_arg), "--manifest=%s", manifest_path.c_str());
        fs::current_path(tmppath);
        argv[2] = manifest_arg;
        ASSERT_EQ(app_main(3, argv), App_Status_Code::OK);
    }

    // Fails if src does not exist
    {
        setup(tmppath);
        defer(teardown(tmppath));
        defer(fs::current_path(initial_path));

        fs::path manifest_path = fs::canonical("../test/integration/testfiles/file_src_not_exist.manifest");

        char manifest_arg[1024];
        snprintf(manifest_arg, sizeof(manifest_arg), "--manifest=%s", manifest_path.c_str());
        fs::current_path(tmppath);
        argv[2] = manifest_arg;
        ASSERT_NE(app_main(3, argv), App_Status_Code::OK);
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}