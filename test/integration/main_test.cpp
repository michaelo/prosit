#include "app.h"
#include "test.h"
#include "itest.h"

#include "mlib/defer.imp.h"

#include <cassert>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

TEST(MainTest, test_https)
{
    ASSERT_EQ(basic_app_main_run("../test/integration/testfiles/empty.manifest"), App_Status_Code::OK);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}