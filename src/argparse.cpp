//
// Basic hand-rolled argparser
//
#include <cstdio>
#include <cstring>
#include <algorithm>

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
    -f, --force    Override in case of destructive actions
        --manifest Override default manifest name/path (default: %s)
    -x, --outoftree     Required for manifests specifying destinations outside of the
                        directory of the manifest
    -s, --silent   Suppress all output except error messages
    -v, --verbose  Show debug output
        --version  Version

(c) Michael Odden - https://github.com/michaelo/prosit
)help",
           APP_NAME, app_version(), APP_NAME, DEFAULT_MANIFEST_NAME);
}

// arguments_out must be freed if function returns true
Argparse_Status cli_argparse(int argc, char **argv, CliArguments **arguments_out)
{
    CliArguments *arguments = new CliArguments;
    defer(delete (arguments));
    memset(arguments, 0, sizeof(CliArguments));
    char scrap[1024];

    // Early opt-out
    if (argc < 2)
    {
        printf("ERROR: Not enough parameters. Try %s --help.\n", argv[0]);
        return Argparse_Status::Error;
    }

    // Check for non-subcommand flags (help, version)
    for (int i = 1; i < argc; i++)
    {
        // Check for -h/--help
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            cli_print_help();
            return Argparse_Status::OkButQuit;
        }

        // Check for --version
        if (strcmp(argv[i], "--version") == 0)
        {
            printf("prosit v%s\n", app_version());
            return Argparse_Status::OkButQuit;
        }
    }

    // Check for sub-command
    if (!deserialize(argv[1], &arguments->command))
    {
        printf("ERROR: Invalid subcommand: %s\n", argv[1]);
        return Argparse_Status::Error;
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

        if (strcmp(argv[i], "-x") == 0 || strcmp(argv[i], "--outoftree") == 0)
        {
            arguments->outoftree = true;
            continue;
        }

        if (strncmp(argv[i], "--manifest=", 11) == 0)
        {
            if (sscanf(argv[i], "--manifest=%1024s", scrap) == 1)
            {
                strncpy(arguments->manifest_path, scrap, std::min(sizeof(scrap), sizeof(arguments->manifest_path)));
            }
            else
            {
                printf("ERROR: Could not parse value of --manifest");
                return Argparse_Status::Error;
            }
            continue;
        }
    }

    *arguments_out = new CliArguments;
    memcpy(*arguments_out, arguments, sizeof(CliArguments));
    return Argparse_Status::Ok;
}
