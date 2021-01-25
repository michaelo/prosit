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
    char* tmppath;

    // Get file from plain http
    // TODO

    // Get unprotected file from https
    {
        App_Status_Code result = basic_app_main_run_no_teardown("../test/integration/testfiles/https.manifest", &tmppath);
        defer({
            teardown(tmppath);
            delete(tmppath);
        });
        ASSERT_EQ(result, App_Status_Code::Ok);
        ASSERT_TRUE(file_exists_in_path(tmppath, "file.txt"));
    }

    // Get basic auth protected http file
    // TODO

    // Get basic auth protected https file
    {
        App_Status_Code result = basic_app_main_run_no_teardown("../test/integration/testfiles/https_basic_auth.manifest", &tmppath);
        defer({
            teardown(tmppath);
            delete(tmppath);
        });
        ASSERT_EQ(result, App_Status_Code::Ok);
        ASSERT_TRUE(file_exists_in_path(tmppath, "file.txt"));
    }

    // Fails if no auth-details provided for auth protected file
    ASSERT_NE(basic_app_main_run("../test/integration/testfiles/https_basic_auth_missing_login.manifest"), App_Status_Code::Ok);

    // Fails if incorrect auth-details provided for auth protected file
    ASSERT_NE(basic_app_main_run("../test/integration/testfiles/https_basic_auth_incorrect_login.manifest"), App_Status_Code::Ok);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}