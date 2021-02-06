#include "app.h"
#include "test.h"
#include "itest.h"

#include "mlib/defer.imp.h"

#include <cassert>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

TEST(HttpsTest, shall_get_unprotected_file)
{
    // Get unprotected file from https
    ASSERT_TRUE(test_succeeds_and_contains_files(R"(
        https: https://src.michaelodden.com/prosit/unprotected/file.txt > file.txt
        )", 1, TESTFILEARR{
            "file.txt"
        }));
}

TEST(HttpsTest, shall_get_basic_auth_protected_file)
{
    // Get basic auth protected https file
    ASSERT_TRUE(test_succeeds_and_contains_files(R"(
        https: https://testuser:testpass@src.michaelodden.com/prosit/basicauth/file.txt > file.txt
        )", 1, TESTFILEARR{
            "file.txt"
        }));
}

TEST(HttpsTest, shall_return_error_on_auth_error)
{
    // Fails if no auth-details provided for auth protected file
    ASSERT_NE(test_allinone(R"(
        https: https://src.michaelodden.com/prosit/basicauth/file.txt > file.txt
        )"), App_Status_Code::Ok);

    // Fails if incorrect auth-details provided for auth protected file
    ASSERT_NE(test_allinone(R"(
        https: https://testuser:incorrectpass@src.michaelodden.com/prosit/basicauth/file.txt > file.txt
        )"), App_Status_Code::Ok);
}


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}