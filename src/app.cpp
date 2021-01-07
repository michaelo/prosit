#include <cstdio>
#include <cstdarg>
#include <cstring>

#include "app.h"

#include "mlib/utils.imp.h"
#include "mlib/defer.imp.h"

#include <filesystem>
namespace fs = std::filesystem;

// Common
bool path_is_relative_inside_workspace(Context* c, const char* path) {
    assert(strlen(path)>1);
    // Return true if:
    // path is not absolute
    // does not traverse upwards
    // To check: starts with .. or / or 
    // Require that cwd of manifest is substring of absolute(path) starting at 0

    if(path[0] == '/') return false;
    if(path[0] == '\\') return false;
    if(path[0] == '.' && path[1] == '.') return false;
    if(path[1] == ':') return false; // Indicative of Win drive-based-path

    // Checks that the resolved manifest+path-path starts with the manifest path
    char joint_path[MAX_PATH_LEN];
    snprintf(joint_path, sizeof(joint_path), "%s/%s", c->manifest_path_abs, path);
    const char* abs_path = fs::absolute(fs::path(joint_path)).c_str();
    if(strstr(abs_path, c->manifest_path_abs) != abs_path) return false; // abs_path doesn't start with manifest_path_abs

    return true;
}

bool precheck_manifest(Context*c, Manifest* m) {
    // Do initial verification of high level correctness of manifest. E.g. with regards to out-of-tree-destination
    (void)c;
    (void)m;
    return false;
}

// Att! Modifies str
void expand_environment_vars(char* str, const size_t str_len) {
    // Strategy:
    // Create a new buffer of same size as str (this is max-capacity)
    // Search for env-patterns: ${MY_VAR} or $(MY_VAR)? Support both? TBD
    // Look up env, if found: inject
    // TODO: Need to copy everything pre-var to buffer, + var-replacement, then continue parsing. May be multiple envs in single string
    // TBD: May continiously manipulate same str for performance. This is not a bottleneck, so we'll start out safe
    assert(str_len < 2048);
    char scrap[2048];
    size_t n = 0; // bytes used for expanded string

    // char* tok = scrap;
    char* sym = sym;

    char* env;

    char symbuf[128];
    bool any_expansions = false;
    for(size_t i=0; i<str_len-1; i++) {
        // Locate start of potential symbol
        if(str[i] == '$' && str[i+1] == '(') {
            sym = str+i+2;

            // Locate end of potential symbol
            // Is there a closing ')' within string?
            for(size_t j=i+1; j<str_len; j++) {
                if(str[j] == ')') {
                    // TODO: set i=j+1 to avoid processing the contents of symbol-like
                    size_t sym_len = str+j-sym; // TODO: fail if too long
                    memcpy(symbuf, sym, sym_len);
                    symbuf[sym_len] = '\0';
                    
                    // printf("Got match: %s\n", symbuf);
                    env = getenv(symbuf);

                    if(env != nullptr) {
                        // printf(" match in env: %s\n", env);
                        any_expansions = true;
                        memcpy(scrap+n, env, strlen(env));

                        n += strlen(env);
                        // printf("n: %d\n", n);
                        i = j;
                    }

                    
                    break;
                }
            }
        } else {
            scrap[n++] = str[i];
        }
    }

    if(any_expansions) {
        // strncpy(str, scrap, strlen(scrap));
        scrap[n] = '\0';
        strcpy(str, scrap);
    }
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

    // TODO: Spread out multithread?
    // Att! Undefined behaviour of multiple entries manipulates the same files/folders
    // TODO: Make singlethreading an option, and automatically determine number of threads for mt from std::thread::hardware_concurrency();
    for(int i=0; i<manifest->length; i++) {
        // TODO: Do initial verification e.g. re dest being inside workspace or not
        c->debug("Processing: '%s' '%s' -> '%s'\n",
               manifest->entries[i].type,
               manifest->entries[i].src,
               manifest->entries[i].dst);

        if(strcmp(manifest->entries[i].type, "git") == 0) {
            handle_git(c, &manifest->entries[i]);
        }
    }

    return Handler_Status::OK;
}

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

void context_init(Context* c) {
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
