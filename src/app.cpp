#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <filesystem>

#include "app.h"

#include "mlib/utils.imp.h"
#include "mlib/defer.imp.h"

namespace fs = std::filesystem;

bool precheck_manifest(Context *c, CliArguments *a, Manifest *m)
{
    // Do initial verification of high level correctness of manifest. E.g. with regards to out-of-tree-destination
    (void)c;
    (void)m;

    bool any_errors = false;;

    Manifest_Entry* entry;
    for (int i = 0; i < m->length; i++)
    {
        entry = &m->entries[i];
        if(!path_is_relative_inside_workspace(c->manifest_path_abs, entry->dst)) {
            if(!a->outoftree) {
                c->error("manifest line: %d: destination may point to outside of project workspace (%s and %s)\n", entry->line_in_manifest, c->manifest_path_abs, entry->dst);
                any_errors = true;
            } else {
                c->debug("manifest line: %d: destination may point to outside of project workspace\n", entry->line_in_manifest);
            }
        }
    }

    return !any_errors;
}


Handler_Status cmd_update(Context *c, CliArguments *a)
{
    (void)c;
    (void)a;

    Manifest *manifest;
    if (!manifest_parse("project.manifest", &manifest))
    {
        c->error("Got error loading manifest - see previous message(s)\n");
        return Handler_Status::Error;
    }
    defer(manifest_free(manifest));

    strcpy(c->manifest_path_abs, fs::absolute(fs::path("project.manifest").parent_path()).c_str());
    printf("workspace: %s\n", c->manifest_path_abs);

    if(!precheck_manifest(c, a, manifest)) {
        return Handler_Status::Error;
    }

    // TODO: Spread out multithread?
    // Att! Undefined behaviour of multiple entries manipulates the same files/folders
    // TODO: Make singlethreading an option, and automatically determine number of threads for mt from std::thread::hardware_concurrency();
    for (int i = 0; i < manifest->length; i++)
    {
        // TODO: Do initial verification e.g. re dest being inside workspace or not
        c->debug("Processing: '%s' '%s' -> '%s' (entry %d, line %d)\n",
                manifest->entries[i].type,
                manifest->entries[i].src,
                manifest->entries[i].dst,
                i+1,
                manifest->entries[i].line_in_manifest);

        if (strcmp(manifest->entries[i].type, "git") == 0) handle_git(c, &manifest->entries[i]);
        if (strcmp(manifest->entries[i].type, "file") == 0) handle_file(c, &manifest->entries[i]);
    }

    return Handler_Status::OK;
}


// TODO: Temporary solution until proper output-strategy is determined
void print_debug(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stdout, "DEBUG: ");
    vfprintf(stdout, format, args);
    va_end(args);
}

void print_info(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stdout, "INFO: ");
    vfprintf(stdout, format, args);
    va_end(args);
}

void print_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stderr, "ERROR: ");
    vfprintf(stderr, format, args);
    va_end(args);
}

void context_init(Context *c)
{
    c->debug = print_debug;
    c->info = print_info;
    c->error = print_error;
}

int app_main(int argc, char **argv)
{
    Context c;
    context_init(&c);

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
        c.error("Unsupported subcommand\n");
        return -1;
        break;
    }

    return 0;
}

const char *app_version()
{
    return "0.1.0"; // TODO: Get from build sys
}


/////////////////////
// Utility-functions
/////////////////////
bool path_is_relative_inside_workspace(const char* workspace_path, const char *path_to_check)
{
    assert(strlen(path_to_check) > 1);
    // Returns false if:
    //    path is not relative
    //    attempts to traverse upwards out of directory of manifest

    if (path_to_check[0] == '/')
        return false;
    if (path_to_check[0] == '\\')
        return false;
    if (path_to_check[0] == '.' && path_to_check[1] == '.')
        return false;
    if (path_to_check[1] == ':')
        return false; // Indicative of Win drive-based-path

    // Checks that the resolved manifest+path-path ends up starting with the manifest path
    //   this to ensure there are no relative-tricks mid-string.
    //   TBD: Might simply be solved by checking for any occurrence of ".."
    char joint_path[MAX_PATH_LEN];
    snprintf(joint_path, sizeof(joint_path), "%s%s", workspace_path, path_to_check);
    fs::path abs = fs::weakly_canonical(joint_path);
    const char *abs_path = abs.c_str();
    if (strstr(abs_path, workspace_path) != abs_path)
    {
        printf("must be here: %s - %s vs %s\n", joint_path, abs_path, workspace_path);
        return false; // abs_path doesn't start with manifest_path_abs
    }

        return true;
    }

// Att! Modifies str. TODO: Test to make in-place to avoid having to create temporary buffer.
void expand_environment_vars(char *str, const size_t str_len)
{
    // Searches for symbols of format $(var_name) and replaces it with the corresponding env-value if found.
    // Strategy: currently using separate buffer to write the expanded string, before copying it to source. 
    // TBD: May continously manipulate same str for performance and avoidance of separate buffer on stack
    assert(str_len < 2048);
    char scrap[2048];
    size_t n = 0; // byte count for expanded string

    char *sym = sym;
    char *env;

    char symbuf[128];
    bool any_expansions = false;
    for (size_t i = 0; i < str_len - 1; i++)
    {
        // Locate start of potential symbol
        if (str[i] == '$' && str[i + 1] == '(')
        {
            sym = str + i + 2;

            // Locate end of potential symbol
            // Find first possible closing ')' within string?
            for (size_t j = i + 1; j < str_len; j++)
            {
                if (str[j] == ')')
                {
                    size_t sym_len = str + j - sym;
                    memcpy(symbuf, sym, sym_len);
                    symbuf[sym_len] = '\0';

                    // printf("Got match: %s\n", symbuf);
                    env = getenv(symbuf);

                    if (env != nullptr)
                    {
                        // printf(" match in env: %s\n", env);
                        size_t env_len = strlen(env);
                        any_expansions = true;
                        memcpy(scrap + n, env, env_len); // Don't want \0

                        n += env_len;
                        assert(n < str_len); // Cheap assurance.
                        i = j;
                    } else {
                        // TODO: Use proper print-function
                        printf("WARNING: Found env-like symbol, but no such env found: %s\n", symbuf);
                    }

                    break;
                }
            }
        }
        else
        {
            scrap[n++] = str[i];
        }
    }

    if (any_expansions)
    {
        scrap[n] = '\0';
        strcpy(str, scrap);
    }
}
