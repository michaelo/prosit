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

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
