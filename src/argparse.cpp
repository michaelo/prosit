//
// Basic hand-rolled argparser
//
// TODO: Generalize for other CLI-apps. Datastructure as template-arg, lambdas as callbacks for argument-matches
//       Features: global flags/args, subcommands, subcommand-args/flags, version, help
#include <cstdio>
#include <cstring>

#include "app.h"

#include "mlib/defer.imp.h"

bool deserialize(char *str, Subcommand *out)
{
    *out = Subcommand::Unspecified;

    if (strcmp(str, "update") == 0)
        *out = Subcommand::Update;

    return *out != Subcommand::Unspecified;
}

void cli_print_help()
{
    printf(R"help(%s v%s

Usage: %s --help|<subcommand> [args]

Subcommands:
    update      Attempt to ensure the workspace is in sync with the manifest

Global args:
    -h, --help     This help
        --version  Version
    -v, --verbose  Show debug output
    -s, --silent   Suppress all output except error messages
    -f, --force    Override in case of destructive actions
    -x, --outoftree     Required for manifests specifying destinations outside of the
                        directory of the manifest

(c) Michael Odden - https://github.com/michaelo/prosit
)help",
           APP_NAME, app_version(), APP_NAME);
}

// arguments_out must be freed if function returns true
bool cli_argparse(int argc, char **argv, CliArguments **arguments_out)
{
    CliArguments *arguments = new CliArguments;
    defer(delete (arguments));
    memset(arguments, 0, sizeof(CliArguments));

    // Early opt-out
    if (argc < 2)
    {
        printf("ERROR: Not enough parameters\n");
        return false;
    }

    // Check for non-subcommand flags (help, version)
    for (int i = 1; i < argc; i++)
    {
        // Check for -h/--help
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            cli_print_help();
            return false;
        }

        // Check for --version
        if (strcmp(argv[i], "--version") == 0)
        {
            printf("prosit v%s\n", app_version());
            return false;
        }
    }

    // Check for sub-command
    if (!deserialize(argv[1], &arguments->command))
    {
        printf("ERROR: Invalid subcommand: %s\n", argv[1]);
        return false;
    }

    // Check for common flags (-f, -v, -s)
    for (int i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--force") == 0)
        {
            arguments->force = true;
            continue;
        }

        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0)
        {
            arguments->verbose = true;
            continue;
        }

        if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--silent") == 0)
        {
            arguments->silent = true;
            continue;
        }

        // TODO: Consider moving to only applicable subcommands
        if (strcmp(argv[i], "-x") == 0 || strcmp(argv[i], "--outoftree") == 0)
        {
            arguments->outoftree = true;
            continue;
        }
    }

    *arguments_out = new CliArguments;
    memcpy(*arguments_out, arguments, sizeof(CliArguments));
    return true;
}
