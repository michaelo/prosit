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
    // Get file from plain http
    // TODO

    // Get unprotected file from https
    ASSERT_EQ(basic_app_main_run("../test/integration/testfiles/https.manifest"), App_Status_Code::OK);

    // Get basic auth protected http file
    // TODO

    // Get basic auth protected https file
    ASSERT_EQ(basic_app_main_run("../test/integration/testfiles/https_basic_auth.manifest"), App_Status_Code::OK);

    // Fails if no auth-details provided for auth protected file
    ASSERT_NE(basic_app_main_run("../test/integration/testfiles/https_basic_auth_missing_login.manifest"), App_Status_Code::OK);

    // Fails if incorrect auth-details provided for auth protected file
    ASSERT_NE(basic_app_main_run("../test/integration/testfiles/https_basic_auth_incorrect_login.manifest"), App_Status_Code::OK);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}