#include "app.h"
#include "test.h"
#include "itest.h"

#include "mlib/defer.imp.h"

#include <cassert>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

TEST(HttpsTest, test_https)
{
    char tmppath[L_tmpnam];
    fs::path initial_path = fs::current_path();

    // TODO: Check resulting contents instead of just the return code.

    char *argv[] = {
        (char *)"prosit",
        (char *)"update",
        (char *)NULL};

    {
        setup(tmppath);
        defer(teardown(tmppath));
        defer(fs::current_path(initial_path));

        fs::path manifest_path = fs::canonical("../test/integration/testfiles/empty.manifest");

        char manifest_arg[1024];
        snprintf(manifest_arg, sizeof(manifest_arg), "--manifest=%s", manifest_path.c_str());
        fs::current_path(tmppath);
        argv[2] = manifest_arg;
        ASSERT_EQ(app_main(3, argv), App_Status_Code::OK);
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
        argv[2] = manifest_arg;
        ASSERT_EQ(app_main(3, argv), App_Status_Code::OK);
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
        argv[2] = manifest_arg;
        ASSERT_EQ(app_main(3, argv), App_Status_Code::OK);
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
        argv[2] = manifest_arg;
        ASSERT_NE(app_main(3, argv), App_Status_Code::OK);
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
        argv[2] = manifest_arg;
        ASSERT_NE(app_main(3, argv), App_Status_Code::OK);
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}