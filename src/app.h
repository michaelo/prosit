#pragma once

#include <cstdio>

static const int MAX_PATH_LEN = 1024;

//////////////////////
// Entry point API
//////////////////////
int app_main(int argc, char** argv);
const char* app_version();

// This might actually make sense to make class of
// Alternatively, use macros which take Context* as parameter
struct Context {
    // 
    char manifest_path_abs[MAX_PATH_LEN]; // abs

    // "Global"-ish data to pass around
    bool verbose = false;
    bool silent = false;

    // Output-functions
    void(*debug)(const char*, ...);
    void(*info)(const char*, ...);
    void(*error)(const char*, ...);
};

//////////////////////
// Manifest
//////////////////////
struct Manifest_Entry {
    int line_in_manifest;
    char type[16];
    char src[MAX_PATH_LEN];
    char dst[MAX_PATH_LEN];
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

    bool verbose; // Including debug-output
    bool silent; // Suppress everything but errors

    bool outoftree;
    bool force;
    // char* cwd;
    // char* manifest_path;
};

bool cli_argparse(int argc, char** argv, CliArguments** arguments_out);

//////////////////////
// Handlers
//////////////////////
enum Handler_Status {
    OK = 0,
    Error,
};

Handler_Status handle_git(Context*, Manifest_Entry*);
Handler_Status handle_file(Context*, Manifest_Entry*);

//////////////////////
// Generic
//////////////////////
bool path_is_relative_inside_workspace(const char* workspace_path, const char *path_to_check);
void expand_environment_vars(char* str, size_t str_len);
