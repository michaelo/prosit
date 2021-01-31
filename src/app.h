#pragma once

#include <cstdio>

static const char APP_NAME[] = "prosit";
static const char DEFAULT_MANIFEST_NAME[] = "prosit.manifest";

static const int MAX_PATH_LEN = 1024;
static const int MAX_TYPE_LEN = 16;

//////////////////////
// Entry point API
//////////////////////
enum class App_Status_Code {
    Ok = 0,
    Error,
};

App_Status_Code app_main(int argc, char** argv);
const char* app_version();

// This might actually make sense to make class of
// Alternatively, use macros which take Context* as parameter
struct Context {
    // The cwd when executing will be treated as the workspace of your project and 
    // serve as a "root"-folder.
    char workspace_path_abs[MAX_PATH_LEN];

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
    char type[MAX_TYPE_LEN];
    char src[MAX_PATH_LEN];
    char dst[MAX_PATH_LEN];
};

struct Manifest {
    int length = 0;
    Manifest_Entry entries[128]; // TODO: Fix to dynamic size based in input data (two-pass parse)
};

bool manifest_parse_buf(char* buf, Manifest **manifest_out);
bool manifest_parse(const char* path, Manifest** manifest_out);
void manifest_free(Manifest* m);

//////////////////////
// Argparse
//////////////////////
enum class Subcommand {
    Unspecified,
    Update
};

struct CliArguments {
    Subcommand command;

    bool verbose; // Including debug-output
    bool silent; // Suppress everything but errors

    bool outoftree;
    bool force;

    char manifest_path[MAX_PATH_LEN]; // <-- buffer or ref? For lifetime's sake; buffer.
};

enum class Argparse_Status {
    Ok,
    OkButQuit, // e.g. for --help or --version
    Error
};

Argparse_Status cli_argparse(int argc, char** argv, CliArguments** arguments_out);

//////////////////////
// Handlers
//////////////////////
App_Status_Code handle_git(Context*, Manifest_Entry*);
App_Status_Code handle_file(Context*, Manifest_Entry*);
App_Status_Code handle_https(Context*, Manifest_Entry*);

//////////////////////
// Generic
//////////////////////
bool path_is_relative_inside_workspace(const char* workspace_path, const char *path_to_check);
void expand_environment_vars(char* str, size_t str_len);
bool extract_login_from_uri(const char* uri, char* username_out, size_t username_len, char* password_out, size_t password_len);
void mask_login_from_uri(char* uri, size_t uri_size);
size_t string_trim(char *str);
