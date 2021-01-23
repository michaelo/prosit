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
    char* tmppath;

    // Verify clone of remote git repo
    {
        App_Status_Code result = basic_app_main_run_no_teardown("../test/integration/testfiles/git_remote.manifest", &tmppath);
        defer({
            teardown(tmppath);
            delete(tmppath);
        });
        ASSERT_EQ(result, App_Status_Code::OK);
        
        ASSERT_TRUE(file_exists_in_path(tmppath, "cloned/to/prosit/.git"));
        ASSERT_TRUE(file_exists_in_path(tmppath, "cloned/to/prosit/README.md"));
    }

    // Verify update existing clone

    // Verify checkout to different branch/tag

    // Verify fail if local folder exists, but is not a rep
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}