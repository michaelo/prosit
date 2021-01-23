#include "app.h"
#include "test.h"
#include "itest.h"

#include "mlib/defer.imp.h"

#include <cassert>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

TEST(FileTest, test_file)
{
    char* tmppath;

    // Copy local file to specific dest name
    {
        App_Status_Code result = basic_app_main_run_no_teardown("../test/integration/testfiles/file.manifest", &tmppath);
        defer({
            teardown(tmppath);
            delete(tmppath);
        });
        ASSERT_EQ(result, App_Status_Code::OK);
        ASSERT_TRUE(file_exists_in_path(tmppath, "dummy.txt"));
        ASSERT_TRUE(file_exists_in_path(tmppath, "folder/dummy.txt"));
    }
    
    
    // Copy local file to folder (keeps original filename)
    {
        App_Status_Code result = basic_app_main_run_no_teardown("../test/integration/testfiles/file_no_dest_name.manifest", &tmppath);
        defer({
            teardown(tmppath);
            delete(tmppath);
        });
        ASSERT_EQ(result, App_Status_Code::OK);
        ASSERT_TRUE(file_exists_in_path(tmppath, "dummy.txt"));
        ASSERT_TRUE(file_exists_in_path(tmppath, "folder/dummy.txt"));
    }

    // Fails if src does not exist
    ASSERT_NE(basic_app_main_run("../test/integration/testfiles/file_src_not_exist.manifest"), App_Status_Code::OK);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}