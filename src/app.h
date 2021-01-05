#pragma once

//////////////////////
// Entry point API
//////////////////////
int app_main(int argc, char** argv);
const char* app_version();

struct Context {
    // "Global"-ish data to pass around
};


//////////////////////
// Handlers
//////////////////////
enum Handler_Status {
    OK,
    Error,
};

Handler_Status handle_git(Context*, char* config);
Handler_Status handle_file(Context*, char* config);

//////////////////////
// Manifest
//////////////////////
struct Manifest_Entry {
    char type[16];
    char src[256];
    char dst[256];
};

struct Manifest {
    int length = 0;
    Manifest_Entry entries[128]; // TODO: Fix to dynamic size based in input data (two-pass parse)
};

bool manifest_parse(const char* path, Manifest** manifest_out);
void manifest_free(Manifest* m);

//////////////////////
// Argparse
//////////////////////
enum Subcommand {
    Unspecified,
    Update
};

struct CliArguments {
    Subcommand command;
    char* cwd;
    char* manifest_path;
};

bool cli_argparse(int argc, char** argv, CliArguments** arguments_out);