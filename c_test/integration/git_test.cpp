#include "app.h"
#include "test.h"
#include "itest.h"

#include "mlib/defer.imp.h"

#include <cassert>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

TEST(GitTest, shall_clone_remote_repo)
{
    // Verify clone of remote git repo
    ASSERT_TRUE(test_succeeds_and_contains_files(R"(
        git: https://github.com/michaelo/_prosit_itest > cloned/to/prosit
        )", 2, TESTFILEARR{
            "cloned/to/prosit/.git",
            "cloned/to/prosit/README.md"
        }));
}

TEST(GitTest, shall_clone_remote_repo_and_switch_branch)
{
    // Verify update existing clone
    Test_Context context;
    test_setup(&context);
    defer(test_teardown(&context));
    
    ASSERT_TRUE(test_run_with_manifest_and_contains_files(&context, R"manifest(
        git: https://github.com/michaelo/_prosit_itest#main > cloned/to/prosit
        )manifest", 2, TESTFILEARR{
            "cloned/to/prosit/.git",
            "cloned/to/prosit/README.md"
        }));

    ASSERT_TRUE(test_run_with_manifest_and_contains_files(&context, R"manifest(
        git: https://github.com/michaelo/_prosit_itest#dev > cloned/to/prosit
        )manifest", 1, TESTFILEARR{
            "cloned/to/prosit/README_devbranch.md"
        }));
}

// Verify fail if local folder exists, but is not a repo / or repo with other remote?


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}