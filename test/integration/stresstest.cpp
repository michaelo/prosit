#include "app.h"
#include "test.h"
#include "itest.h"

#include "mlib/defer.imp.h"

#include <cassert>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

TEST(MainTest, stresstest)
{
    ASSERT_EQ(basic_app_main_run("test/integration/testfiles/stresstest.manifest"), App_Status_Code::Ok);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}