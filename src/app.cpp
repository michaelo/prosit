#include <cstdio>

#include "app.h"

#include "mlib/utils.imp.h"
#include "mlib/defer.imp.h"

void cmd_update(Context* c, CliArguments* a) {
    (void)c;
    (void)a;
}

int app_main(int argc, char** argv) {
    CliArguments* args;
    bool argparse_result = cli_argparse(argc, argv, &args);
    defer(if(argparse_result) delete(args));

    
    // Read manifest? Or to be handled by command handlers that need it? For now: lazy, ie. in cmd-handler
    // Parse arguments
    // Establish context
    // Send context + parsed manifest to appropriate command handler (update ... etc)
    return 0;
}

const char* app_version() {
    return "0.1.0"; // TODO: Get from build sys
}
