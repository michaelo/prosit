#include "app.h"
#include "test.h"

TEST(ArgparseTest, Parse)
{
    {
        CliArguments *result;
        char *argv[] = {(char *)"prosit"};

        ASSERT_FALSE(cli_argparse(1, argv, &result));
    }

    {
        CliArguments *result;
        char *argv[] = {
            (char *)"prosit",
            (char *)"update"};

        ASSERT_TRUE(cli_argparse(2, argv, &result));
        ASSERT_EQ(result->command, Subcommand::Update);

        ASSERT_FALSE(result->verbose);
        ASSERT_FALSE(result->silent);
        ASSERT_FALSE(result->force);
        ASSERT_FALSE(result->outoftree);
    }

    {
        CliArguments *result;
        char *argv[] = {
            (char *)"prosit",
            (char *)"--help"};

        ASSERT_FALSE(cli_argparse(2, argv, &result));
    }

    {
        CliArguments *result;
        char *argv[] = {
            (char *)"prosit",
            (char *)"update",
            (char *)"--verbose"};

        ASSERT_TRUE(cli_argparse(3, argv, &result));
        ASSERT_TRUE(result->verbose);
    }

    {
        CliArguments *result;
        char *argv[] = {
            (char *)"prosit",
            (char *)"update",
            (char *)"--silent"};

        ASSERT_TRUE(cli_argparse(3, argv, &result));
        ASSERT_TRUE(result->silent);
    }

    {
        CliArguments *result;
        char *argv[] = {
            (char *)"prosit",
            (char *)"update",
            (char *)"--outoftree"};

        ASSERT_TRUE(cli_argparse(3, argv, &result));
        ASSERT_TRUE(result->outoftree);
    }

    {
        CliArguments *result;
        char *argv[] = {
            (char *)"prosit",
            (char *)"update",
            (char *)"--force"};

        ASSERT_TRUE(cli_argparse(3, argv, &result));
        ASSERT_TRUE(result->force);
    }

    {
        CliArguments *result;
        char *argv[] = {
            (char *)"prosit",
            (char *)"update",
            (char *)"--manifest=custom.project"};

        ASSERT_TRUE(cli_argparse(3, argv, &result));
        ASSERT_STREQ(result->manifest_path, "custom.project");
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
