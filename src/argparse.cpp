#include <cstdio>
#include <cstring>

#include "app.h"

#include "mlib/utils.imp.h"
#include "mlib/defer.imp.h"

bool deserialize(char* str, Subcommand *out) {
    *out = Subcommand::Unspecified;
    
    if(strcmp(str, "update")==0) *out = Subcommand::Update;

    return *out != Subcommand::Unspecified;
}

// arguments_out must be freed if function returns true
bool cli_argparse(int argc, char** argv, CliArguments** arguments_out) {
    CliArguments* arguments = new CliArguments;
    defer(delete(arguments));

    if(argc < 2) {
        printf("ERROR: Not enough parameters\n");
        return false;
    }

    if(!deserialize(argv[1], &arguments->command)) {
        printf("ERROR: Invalid subcommand: %s\n", argv[1]);
        return false;
    }

    *arguments_out = new CliArguments;
    memcpy(*arguments_out, arguments, sizeof(CliArguments));
    return true;
}
