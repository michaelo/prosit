#include "app.h"
#include "test.h"
#include "itest.h"

#include "mlib/defer.imp.h"

#include <cassert>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

TEST(HgTest, shall_clone_local_repo)
{
    ASSERT_TRUE(test_succeeds_and_contains_files(R"(
        hg: $(PROSIT_ITEST_TESTFILES)/dummy.hg#main > cloned/to/hgdummy
        )", 2, TESTFILEARR{
            "cloned/to/hgdummy/.hg",
            "cloned/to/hgdummy/README.md"
        }));
}

TEST(HgTest, shall_clone_local_repo_and_switch_branch)
{
    // Verify update existing clone
    Test_Context context;
    test_setup(&context);
    defer(test_teardown(&context));
    // const char manifest[] = R"manifest()manifest";
    ASSERT_TRUE(test_run_with_manifest_and_contains_files(&context, R"manifest(
        hg: $(PROSIT_ITEST_TESTFILES)/dummy.hg#main > cloned/to/hgdummy
        )manifest", 2, TESTFILEARR{
            "cloned/to/hgdummy/.hg",
            "cloned/to/hgdummy/README.md"
        }));

    ASSERT_TRUE(test_run_with_manifest_and_contains_files(&context, R"manifest(
        hg: $(PROSIT_ITEST_TESTFILES)/dummy.hg#dev > cloned/to/hgdummy
        )manifest", 1, TESTFILEARR{
            "cloned/to/hgdummy/README_devbranch.md"
        }));
}

// Verify fail if local folder exists, but is not a repo / or repo with other remote?



int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}