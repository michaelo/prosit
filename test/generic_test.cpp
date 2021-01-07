#include <cstdlib>
#include <filesystem>

#include "app.h"
#include "test.h"

#include "platform.h"

namespace fs = std::filesystem;

TEST(GenericTest, path_is_relative_inside_workspace)
{
    Context c;
    fs::path manifest_path("/my/repo/project.manifest");
    
    strncpy(c.manifest_path_abs, (const char*)fs::absolute(manifest_path.parent_path()).c_str(), sizeof(c.manifest_path_abs));
    ASSERT_TRUE(path_is_relative_inside_workspace(c.manifest_path_abs, "./ok"));
    ASSERT_TRUE(path_is_relative_inside_workspace(c.manifest_path_abs, "ok"));
    ASSERT_FALSE(path_is_relative_inside_workspace(c.manifest_path_abs, "/not/ok"));
    ASSERT_FALSE(path_is_relative_inside_workspace(c.manifest_path_abs, "../not/ok"));
    ASSERT_FALSE(path_is_relative_inside_workspace(c.manifest_path_abs, "c:\\not\\ok"));
    ASSERT_FALSE(path_is_relative_inside_workspace(c.manifest_path_abs, "e:\\not\\ok"));
    ASSERT_FALSE(path_is_relative_inside_workspace(c.manifest_path_abs, "\\not\\ok"));
    ASSERT_FALSE(path_is_relative_inside_workspace(c.manifest_path_abs, "trying/../../to/fool/you"));
}

TEST(GenericTest, expand_environment_vars)
{
    setenv("PROSIT_MYVAR", "42", 1);
    char myvar[128]; // Need to be big enough to room expanded string
    {
        strncpy(myvar, "no env here", sizeof(myvar));
        expand_environment_vars(myvar, sizeof(myvar));
        ASSERT_STREQ(myvar, "no env here");
    }

    {
        strncpy(myvar, "$(PROSIT_MYVAR)", sizeof(myvar));
        expand_environment_vars(myvar, sizeof(myvar));
        ASSERT_STREQ(myvar, "42");
    }

    {
        strncpy(myvar, "pre/$(PROSIT_MYVAR)/$(PROSIT_MYVAR)/post", sizeof(myvar));
        expand_environment_vars(myvar, sizeof(myvar));
        ASSERT_STREQ(myvar, "pre/42/42/post");
    }

    {
        strncpy(myvar, "$(PROSIT_MYVAR_NOTSET)", sizeof(myvar));
        expand_environment_vars(myvar, sizeof(myvar));
        ASSERT_STREQ(myvar, "$(PROSIT_MYVAR_NOTSET)");
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
