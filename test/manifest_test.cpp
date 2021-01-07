#include "app.h"
#include "test.h"

TEST(ManifestTest, ParseEmpty)
{
    Manifest *m;
    ASSERT_TRUE(manifest_parse("../test/testfiles/empty.manifest", &m));
    ASSERT_EQ(m->length, 0);
}

TEST(ManifestTest, ParseEntries)
{
    {
        Manifest *m;
        ASSERT_TRUE(manifest_parse("../test/testfiles/single_local-git.manifest", &m));
        ASSERT_EQ(m->length, 1);
        ASSERT_STREQ(m->entries[0].type, "git");
        manifest_free(m);
    }

    {
        Manifest *m;
        ASSERT_TRUE(manifest_parse("../test/testfiles/multi_with_comment.manifest", &m));
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
        char buf[] = R"manifest(
            git: $(PROSIT_FAKEUSER):$(PROSIT_FAKEPASS) > local/$(PROSIT_FAKEUSER)
        )manifest";

        ASSERT_TRUE(manifest_parse_buf(buf, &m));
        ASSERT_STREQ(m->entries[0].src, "mimo:secret");
        ASSERT_STREQ(m->entries[0].dst, "local/mimo");

        manifest_free(m);
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
