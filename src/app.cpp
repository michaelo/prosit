#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <filesystem>

#include "app.h"
#include <prosit_config.h>

#include "mlib/utils.imp.h"
#include "mlib/defer.imp.h"

namespace fs = std::filesystem;

static struct
{
    // TODO: We must provide locking on actual *out when we go mt. Revise. 
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
    {.type = "https",
     .handler = handle_https},
    {.type = "git",
     .handler = handle_git},
    {.type = "file",
     .handler = handle_file}};

// Does initial verification of high level correctness of manifest. E.g. with regards to out-of-tree-destination
bool precheck_manifest(Context *c, CliArguments *a, Manifest *m)
{
    bool any_errors = false;

    Manifest_Entry *entry;
    for (int i = 0; i < m->length; i++)
    {
        entry = &m->entries[i];
        if (!path_is_relative_inside_workspace(c->manifest_path_abs, entry->dst))
        {
            if (!a->outoftree)
            {
                c->error("manifest line: %d: destination may point to outside of project workspace (%s and %s)\n", entry->line_in_manifest, c->manifest_path_abs, entry->dst);
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

// Main entry point for the subcommand "update"
//      Parse manifest
//      For each entry in manifest, execute the type-specific handle_<type>-function
App_Status_Code cmd_update(Context *c, CliArguments *a)
{
    Manifest *manifest;
    if (!manifest_parse(DEFAULT_MANIFEST_NAME, &manifest))
    {
        c->error("Got error loading manifest - see previous message(s)\n");
        return App_Status_Code::Error;
    }
    defer(manifest_free(manifest));

    strcpy(c->manifest_path_abs, (const char *)fs::absolute(fs::path(DEFAULT_MANIFEST_NAME).parent_path()).c_str());
    c->info("workspace: %s\n", c->manifest_path_abs);

    if (!precheck_manifest(c, a, manifest))
    {
        return App_Status_Code::Error;
    }

    // TODO: Spread out multithread?
    // Att! Undefined behaviour of multiple entries manipulates the same files/folders
    // TODO: Make singlethreading an option, and automatically determine number of threads for mt from std::thread::hardware_concurrency();
    for (int i = 0; i < manifest->length; i++)
    {
        // c->info("Processing: '%s' '%s' -> '%s' (entry %d, line %d)\n",
        // Att! Not showing src now as it may contain expanded environment-variables with secrets
        // TODO: For better output we might instead keep both strings. Or implement a masking-function.
        c->info("Processing: '%s' -> '%s' (entry %d, line %d)\n",
                 manifest->entries[i].type,
                 // manifest->entries[i].src,
                 manifest->entries[i].dst,
                 i + 1,
                 manifest->entries[i].line_in_manifest);

        bool handler_found = false;
        for (size_t j = 0; j < sizeof(handlers) / sizeof(handlers[0]); j++)
        {
            if (strcmp(manifest->entries[i].type, handlers[j].type) == 0)
            {
                handlers[j].handler(c, &manifest->entries[i]);
                handler_found = true;
                break;
            }
        }

        if (!handler_found)
        {
            c->error("Unsupported handler type: %s  (entry %d, line %d)\n",
                     manifest->entries[i].type,
                     i + 1,
                     manifest->entries[i].line_in_manifest);
        }
    }

    return App_Status_Code::OK;
}

static void print_debug(const char *format, ...)
{
    if (!OutputStreams.debug)
        return;
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

    // Configure output-handling based on cli-args
    OutputStreams.debug = args->verbose ? stdout : nullptr;
    OutputStreams.info = args->silent ? nullptr : stdout;

    // Check command and start processing
    switch (args->command)
    {
    case Update:
        return (int)cmd_update(&c, args);
        break;
    default:
        // Should not happen as this is already handled in cli_argparse
        c.error("Unsupported subcommand\n");
        return App_Status_Code::Error;
        break;
    }

    return App_Status_Code::OK;
}

const char *app_version()
{
    return APP_VERSION;
}
