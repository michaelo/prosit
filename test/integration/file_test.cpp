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
    // Copy local file to specific dest name
    ASSERT_EQ(basic_app_main_run("../test/integration/testfiles/file.manifest"), App_Status_Code::OK);
    
    // Copy local file to folder (keeps original filename)
    ASSERT_EQ(basic_app_main_run("../test/integration/testfiles/file_no_dest_name.manifest"), App_Status_Code::OK);

    // Fails if src does not exist
    ASSERT_NE(basic_app_main_run("../test/integration/testfiles/file_src_not_exist.manifest"), App_Status_Code::OK);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}