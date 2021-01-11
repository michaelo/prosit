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
    // Verify clone of remote git repo
    ASSERT_EQ(basic_app_main_run("../test/integration/testfiles/git_remote.manifest"), App_Status_Code::OK);

    // Verify update existing clone

    // Verify checkout to different branch/tag

    // Verify fail if local folder exists, but is not a rep
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}