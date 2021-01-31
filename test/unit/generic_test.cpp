#include <cstdlib>
#include <filesystem>

#include "app.h"
#include "test.h"

#include "platform.h"

namespace fs = std::filesystem;

TEST(GenericTest, path_is_relative_inside_workspace)
{
    Context c;
    fs::path manifest_path("/my/repo/prosit.manifest");
    
    strncpy(c.workspace_path_abs, (const char*)fs::absolute(manifest_path.parent_path()).c_str(), sizeof(c.workspace_path_abs));
    ASSERT_TRUE(path_is_relative_inside_workspace(c.workspace_path_abs, "./ok"));
    ASSERT_TRUE(path_is_relative_inside_workspace(c.workspace_path_abs, "ok"));
    ASSERT_FALSE(path_is_relative_inside_workspace(c.workspace_path_abs, "/not/ok"));
    ASSERT_FALSE(path_is_relative_inside_workspace(c.workspace_path_abs, "../not/ok"));
    ASSERT_FALSE(path_is_relative_inside_workspace(c.workspace_path_abs, "c:\\not\\ok"));
    ASSERT_FALSE(path_is_relative_inside_workspace(c.workspace_path_abs, "e:\\not\\ok"));
    ASSERT_FALSE(path_is_relative_inside_workspace(c.workspace_path_abs, "\\not\\ok"));
    ASSERT_FALSE(path_is_relative_inside_workspace(c.workspace_path_abs, "trying/../../to/fool/you"));
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

TEST(GenericTest, extract_login_from_uri)
{
    char input1[] = "http://plain.com/"; // no user/pass
    char input2[] = "http://user1:pass1@plain.com/"; // got user/pass
    char input2s[] = "https://user2:pass2@plain.com/"; // got user/pass
    char input3[] = "http://user@plain.com/"; // incomplete, error

    ASSERT_FALSE(extract_login_from_uri(input1, NULL, 0, NULL, 0));
    char username[32];
    char password[32];
    ASSERT_TRUE(extract_login_from_uri(input2, username, sizeof(username), password, sizeof(password)));
    ASSERT_STREQ(username, "user1");
    ASSERT_STREQ(password, "pass1");
    ASSERT_TRUE(extract_login_from_uri(input2s, username, sizeof(username), password, sizeof(password)));
    ASSERT_STREQ(username, "user2");
    ASSERT_STREQ(password, "pass2");
    ASSERT_FALSE(extract_login_from_uri(input3, NULL, 0, NULL, 0));
}

TEST(GenericTest, mask_login_from_uri)
{
    char input1[] = "http://plain.com/"; // no user/pass
    char input2[] = "https://user2:pass2@plain.com/"; // got user/pass
    char input3[] = "https://plain.com:123/hei@deg"; // no user/pass at proper location

    mask_login_from_uri(input1, sizeof(input1));
    ASSERT_STREQ(input1, "http://plain.com/");
    mask_login_from_uri(input2, sizeof(input2));
    ASSERT_STREQ(input2, "https://*:*@plain.com/");
    mask_login_from_uri(input3, sizeof(input3));
    ASSERT_STREQ(input3, "https://plain.com:123/hei@deg");
}

TEST(GenericTest, string_trim)
{
    char inputs[][16] = {
        "str",
        "str  ",
        "  str",
        "  str  ",
    };

    for(size_t i=0; i<sizeof(inputs)/sizeof(inputs[0]); i++) {
        printf("testing: '%s'\n", inputs[i]);
        ASSERT_EQ(string_trim(inputs[i]), 3);
        ASSERT_STREQ(inputs[i], "str");
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
