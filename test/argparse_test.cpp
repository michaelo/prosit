#include "app.h"
#include "test.h"

TEST(ArgparseTest, Parse)
{
    {
        CliArguments *result;
        char *argv[] = { (char *)"prosit" };

        ASSERT_FALSE(cli_argparse(1, argv, &result));
    }

    {
        CliArguments *result;
        char *argv[] = {
            (char *)"prosit",
            (char *)"update"};

        ASSERT_TRUE(cli_argparse(2, argv, &result));
        ASSERT_EQ(result->command, Subcommand::Update);
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
