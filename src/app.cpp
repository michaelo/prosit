#include <cstdio>

#include "app.h"

#include "mlib/utils.imp.h"
#include "mlib/defer.imp.h"

Handler_Status cmd_update(Context *c, CliArguments *a)
{
    (void)c;
    (void)a;

    Manifest *manifest;
    if (!manifest_parse("project.manifest", &manifest))
    {
        printf("ERROR: Got error loading manifest - see previous message(s)\n");
        return Handler_Status::Error;
    }
    defer(manifest_free(manifest));


    for(int i=0; i<manifest->length; i++) {
        printf("Processing: '%s' '%s' -> '%s'\n",
               manifest->entries[i].type,
               manifest->entries[i].src,
               manifest->entries[i].dst);

        if(strcmp(manifest->entries[i].type, "git") == 0) {
            handle_git(c, &manifest->entries[i]);
        }
    }

    return Handler_Status::OK;
}

int app_main(int argc, char **argv)
{
    Context c;
    CliArguments *args;
    bool argparse_result = cli_argparse(argc, argv, &args);
    defer(if (argparse_result) delete (args));

    if (!argparse_result)
    {
        return -1;
    }

    switch (args->command)
    {
    case Update:
        return (int)cmd_update(&c, args);
        break;
    default:
        printf("ERROR: Unsupported subcommand\n");
        return -1;
        break;
    }

    return 0;
}

const char *app_version()
{
    return "0.1.0"; // TODO: Get from build sys
}
