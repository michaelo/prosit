#include "app.h"
#include "test.h"

#include "mlib/defer.imp.h"
#include "platform.h"


TEST(ManifestTest, ParseEmpty)
{
    char manifest_empty[] = "";
    Manifest *m;
    ASSERT_TRUE(manifest_parse_buf(manifest_empty, &m));
    defer(manifest_free(m));
    ASSERT_EQ(m->length, 0);
}

TEST(ManifestTest, FailOnGarbage)
{
    char manifest[] = "asdfa";
    Manifest *m;
    ASSERT_FALSE(manifest_parse_buf(manifest, &m));
}

TEST(ManifestTest, ParseEntries)
{
    {
        char manifest_single_local_git[] = ""
R"manifest(# git-repo from local path
git: /my/remote/repo.git#main > ./path/to/local/clone
)manifest";

        Manifest *m;
        ASSERT_TRUE(manifest_parse_buf(manifest_single_local_git, &m));
        ASSERT_EQ(m->length, 1);
        ASSERT_STREQ(m->entries[0].type, "git");
        manifest_free(m);
    }

    {
        char manifest_multi_with_comments[] = ""
R"manifest(# Comment here
file: /my/file.txt > ./local/path/to/file.txt

# Other comment here
git: https://localhost:8999/dummy.git#tagv1 > ./path/to/local/clone)manifest";

        Manifest *m;
        ASSERT_TRUE(manifest_parse_buf(manifest_multi_with_comments, &m));
        ASSERT_EQ(m->length, 2);
        ASSERT_STREQ(m->entries[0].type, "file");
        ASSERT_STREQ(m->entries[1].type, "git");
        manifest_free(m);
    }
}

TEST(ManifestTest, EntriesShallSupportEnvVariables)
{
    {
        Manifest *m;
        char buf[] = R"manifest(
            git: src > dest
        )manifest";

        ASSERT_TRUE(manifest_parse_buf(buf, &m));
        ASSERT_STREQ(m->entries[0].src, "src");
        ASSERT_STREQ(m->entries[0].dst, "dest");

        manifest_free(m);
    }

    {
        Manifest *m;
        setenv("PROSIT_FAKEUSER", "mimo", 1);
        setenv("PROSIT_FAKEPASS", "secret", 1);
        char buf[] =
R"manifest(
git: $(PROSIT_FAKEUSER):$(PROSIT_FAKEPASS) > local/$(PROSIT_FAKEUSER)
)manifest";

        ASSERT_TRUE(manifest_parse_buf(buf, &m));
        ASSERT_STREQ(m->entries[0].type, "git");
        ASSERT_STREQ(m->entries[0].src, "mimo:secret");
        ASSERT_STREQ(m->entries[0].dst, "local/mimo");

        manifest_free(m);
    }
}

TEST(ManifestTest, DebugEnvVariableIssueWithLongMaxText)
{
    printf("passed line: %d\n", __LINE__);
    {
        Manifest *m;
        char buf[] = R"manifest(
            file: $(PROSIT_ITEST_TESTFILES)/dummy.txt > dummy.txt
)manifest";

        setenv("PROSIT_ITEST_TESTFILES", "/Users/michaelodden/dev/prosit/test/integration/testfiles", 1);
        if(manifest_parse_buf(buf, &m)) {
            ASSERT_STREQ(m->entries[0].type, "file");
            ASSERT_STREQ(m->entries[0].src, "/Users/michaelodden/dev/prosit/test/integration/testfiles/dummy.txt");
            ASSERT_STREQ(m->entries[0].dst, "dummy.txt");

            manifest_free(m);
        }
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
