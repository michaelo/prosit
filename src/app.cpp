#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <functional>
#include <mutex>

#include "app.h"
#include <prosit_config.h>

#include "mlib/utils.imp.h"
#include "mlib/defer.imp.h"

namespace fs = std::filesystem;

static struct
{
    // TODO: We must provide locking on actual *out when we go mt. Revise.
    std::mutex lock;
    FILE *debug = stdout;
    FILE *info = stdout;
    FILE *error = stderr;
} OutputStreams;

struct Handler_Definition
{
    const char type[16];
    std::function<App_Status_Code(Context *, Manifest_Entry *)> handler;
};

static Handler_Definition handlers[] = {
        {"https", handle_https},
        {"git", handle_git},
        {"file", handle_file}};

// Does initial verification of high level correctness of manifest. E.g. with regards to out-of-tree-destination
bool precheck_manifest(Context *c, CliArguments *a, Manifest *m)
{
    bool any_errors = false;

    Manifest_Entry *entry;
    for (int i = 0; i < m->length; i++)
    {
        entry = &m->entries[i];
        if (!path_is_relative_inside_workspace(c->workspace_path_abs, entry->dst))
        {
            if (!a->outoftree)
            {
                c->error("manifest line: %d: destination may point to outside of project workspace (%s and %s)\n", entry->line_in_manifest, c->workspace_path_abs, entry->dst);
                any_errors = true;
            }
            else
            {
                c->debug("manifest line: %d: destination may point to outside of project workspace\n", entry->line_in_manifest);
            }
        }
    }

    return !any_errors;
}

bool cmd_update_process_entry(Context *c, Manifest_Entry *entry)
{
    // src may contain username/password, this must be somehow masked
    // Current strategy is to try to mask out the appropriate chunk of the URI (assumed)
    // Alternative is to maintain the not-env-variable-expanded manifest-strings, but this
    // also removes the usefulness of verifying that any other env-variables are correct.
    char tmp_src_masked[128];
    strncpy((char *)tmp_src_masked, entry->src, sizeof(tmp_src_masked));
    mask_login_from_uri(tmp_src_masked, sizeof(tmp_src_masked));
    c->info("Processing: '%s' '%s' -> '%s' (line %d)\n",
            entry->type,
            tmp_src_masked,
            entry->dst,
            entry->line_in_manifest);

    // Handler-handling: Check if handler exists, if so: execute the appropriate function
    for (size_t j = 0; j < sizeof(handlers) / sizeof(handlers[0]); j++)
    {
        if (strcmp(entry->type, handlers[j].type) == 0)
        {
            return (handlers[j].handler(c, entry) == App_Status_Code::Ok);
        }
    }

    c->error("Unsupported handler type: %s  (line %d)\n",
             entry->type,
             entry->line_in_manifest);
    return false;
}

// Main entry point for the subcommand "update"
//      Parse manifest
//      For each entry in manifest, execute the type-specific handle_<type>-function
App_Status_Code cmd_update(Context *c, CliArguments *a)
{
    Manifest *manifest;
    if (!manifest_parse(a->manifest_path, &manifest))
    {
        c->error("Got error loading manifest - see previous message(s)\n");
        return App_Status_Code::Error;
    }
    defer(manifest_free(manifest));

    strcpy(c->workspace_path_abs, (const char *)fs::absolute(fs::current_path()).u8string().c_str());
    c->info("workspace: %s\n", c->workspace_path_abs);
    c->info("manifest: %s\n", a->manifest_path);

    if (!precheck_manifest(c, a, manifest))
    {
        return App_Status_Code::Error;
    }

    // Att! Undefined behaviour of multiple entries manipulates the same files/folders
    bool all_ok = true;
    for (int i = 0; i < manifest->length; i++)
    {
        if (!cmd_update_process_entry(c, &manifest->entries[i]))
        {
            all_ok = false;
        }
    }

    return all_ok ? App_Status_Code::Ok : App_Status_Code::Error;
}

static void print_debug(const char *format, ...)
{
    if (!OutputStreams.debug)
        return;

    OutputStreams.lock.lock();
    defer(OutputStreams.lock.unlock());

    va_list args;
    va_start(args, format);
    fprintf(OutputStreams.debug, "DEBUG: ");
    vfprintf(OutputStreams.debug, format, args);
    va_end(args);
}

static void print_info(const char *format, ...)
{
    if (!OutputStreams.info)
        return;

    OutputStreams.lock.lock();
    defer(OutputStreams.lock.unlock());

    va_list args;
    va_start(args, format);
    fprintf(OutputStreams.info, "INFO: ");
    vfprintf(OutputStreams.info, format, args);
    va_end(args);
}

static void print_error(const char *format, ...)
{
    if (!OutputStreams.error)
        return;

    OutputStreams.lock.lock();
    defer(OutputStreams.lock.unlock());

    va_list args;
    va_start(args, format);
    fprintf(OutputStreams.error, "ERROR: ");
    vfprintf(OutputStreams.error, format, args);
    va_end(args);
}

void context_init(Context *c)
{
    c->debug = print_debug;
    c->info = print_info;
    c->error = print_error;
}

// Main library entry point
App_Status_Code app_main(int argc, char **argv)
{
    Context c;
    context_init(&c);

    CliArguments *args;
    Argparse_Status argparse_result = cli_argparse(argc, argv, &args);
    defer(if (argparse_result == Argparse_Status::Ok) delete (args));

    switch (argparse_result)
    {
    case Argparse_Status::Error:
        return App_Status_Code::Error;
        break;
    case Argparse_Status::OkButQuit:
        return App_Status_Code::Ok;
        break;
    default:
        // Do nothing
        break;
    }

    if (strlen(args->manifest_path) == 0)
    {
        strncpy(args->manifest_path, DEFAULT_MANIFEST_NAME, sizeof(args->manifest_path));
    }

    // Configure output-handling based on cli-args
    OutputStreams.debug = args->verbose ? stdout : nullptr;
    OutputStreams.info = args->silent ? nullptr : stdout;

    // Only the context is passed on to most places
    c.silent = args->silent;
    c.verbose = args->verbose;

    // Check command and start processing
    switch (args->command)
    {
    case Subcommand::Update:
        return cmd_update(&c, args);
        break;
    default:
        // Should not happen as this is already handled in cli_argparse
        c.error("Unsupported subcommand\n");
        return App_Status_Code::Error;
        break;
    }

    return App_Status_Code::Ok;
}

const char *app_version()
{
    return APP_VERSION;
}
