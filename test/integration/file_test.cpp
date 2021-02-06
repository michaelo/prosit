#include "app.h"
#include "test.h"
#include "itest.h"

#include "mlib/defer.imp.h"

#include <cassert>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

TEST(FileTest, shall_copy_to_specific_file_name)
{
    Test_Context context;
    test_setup(&context);
    defer(test_teardown(&context));

    ASSERT_TRUE(test_run_with_manifest_and_contains_files(&context, R"(
        file: $(PROSIT_ITEST_TESTFILES)/dummy.txt > dummy.txt
        file: $(PROSIT_ITEST_TESTFILES)/dummy.txt > folder/dummy.txt
    )", 2, 
        TESTFILEARR{
            "dummy.txt",
            "folder/dummy.txt"
        }));
}

TEST(FileTest, shall_copy_to_specific_folder_and_keep_source_name)
{
    Test_Context context;
    test_setup(&context);
    defer(test_teardown(&context));

    ASSERT_TRUE(test_run_with_manifest_and_contains_files(&context, R"(
file: $(PROSIT_ITEST_TESTFILES)/dummy.txt > ./
file: $(PROSIT_ITEST_TESTFILES)/dummy.txt > folder/
    )", 2, 
        TESTFILEARR{
            "dummy.txt",
            "folder/dummy.txt"
        }));
}

TEST(FileTest, shall_fail_if_src_doesnt_exist)
{
    // Fails if src does not exist
    ASSERT_NE(test_allinone(R"manifest(
file: /no/such/file/can/possibly/exist > file.txt
    )manifest"), App_Status_Code::Ok);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}