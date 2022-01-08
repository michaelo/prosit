#include <cassert>
#include <cstring>
#include <filesystem>
#include <algorithm>

#include "app.h"

namespace fs = std::filesystem;

bool path_is_relative_inside_workspace(const char *workspace_path, const char *path_to_check)
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
    snprintf(joint_path, sizeof(joint_path), "%s/%s", workspace_path, path_to_check);
    fs::path abs = fs::weakly_canonical(joint_path);
    auto abs_path_str = abs.u8string();
    const char *abs_path = (const char *)abs_path_str.c_str();
    if (strstr(abs_path, workspace_path) != abs_path)
    {
        return false; // abs_path doesn't start with manifest_path_abs
    }

    return true;
}

// Att! Modifies str. TODO: Test to make in-place to avoid having to create temporary buffer.
void expand_environment_vars(char *str, const size_t str_size)
{
    // Searches for symbols of format $(var_name) and replaces it with the corresponding env-value if found.
    // Strategy: currently using separate buffer to write the expanded string, before copying it to source.
    // TBD: May continously manipulate same str for performance and avoidance of separate buffer on stack
    static const int SCRAP_LEN = MAX_PATH_LEN+1;
    assert(str_size <= SCRAP_LEN);
    char scrap[SCRAP_LEN];
    size_t n = 0; // byte count for expanded string
    size_t str_len = strlen(str);

    char *sym;
    char *env;

    char symbuf[128];
    bool any_expansions = false;
    for (size_t i = 0; i < str_len; i++)
    {
        // Locate start of potential symbol
        if (str[i] == '$' && str[i + 1] == '(')
        {
            sym = str + i + 2;

            // Locate end of potential symbol
            // Find first possible closing ')' within string?
            for (size_t j = i + 1; j < str_size; j++)
            {
                if (str[j] == ')')
                {
                    size_t sym_len = str + j - sym;
                    sym_len = std::min(sym_len, sizeof(symbuf)-1);
                    memcpy(symbuf, sym, sym_len);
                    symbuf[sym_len] = '\0';

                    env = getenv(symbuf);

                    if (env != nullptr)
                    {
                        size_t env_len = strlen(env);
                        any_expansions = true;
                        memcpy(scrap + n, env, env_len); // Don't want \0

                        n += env_len;
                        assert(n < str_size); // Cheap overflow assurance.
                        i = j;
                    }
                    else
                    {
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

// Extracts (if found) username and password from a URI. Returns true if both are found.
// Limitations: protocol must be <= 16 chars, username and password <= 128 chars each. (TODO)
bool extract_login_from_uri(const char *uri, char *username_out, size_t username_len, char *password_out, size_t password_len)
{
    char protocol[16];
    char username_buf[128] = {0};
    char password_buf[128] = {0};
    int matches = sscanf(uri, "%16[^:]://%128[^:]:%128[^@]@", protocol, username_buf, password_buf);

    if (matches != 3)
    {
        return false;
    }

    if (username_out)
        strncpy(username_out, username_buf, std::min(sizeof(username_buf), username_len));
    if (password_out)
        strncpy(password_out, password_buf, std::min(sizeof(password_buf), password_len));

    return true;
}

void mask_login_from_uri(char *uri, size_t uri_size)
{
    int sym_after_double_dash = 0;
    int sym_at = 0;
    int sym_host_end = 0;
    for(int i=1; i<(int)uri_size; i++) {
        if(uri[i] == '/' && uri[i-1] == '/') {
            sym_after_double_dash = i+1;
            continue;
        }

        if(sym_after_double_dash && uri[i] == '/') {
            sym_host_end = i;
            break;
        }

        if(sym_after_double_dash && !sym_host_end && uri[i] == '@') {
            sym_at = i;
            continue;
        }
    }

    // Replace the assumed auth-chunk with "*:*"
    if(sym_after_double_dash && sym_at) {
        snprintf(&uri[sym_after_double_dash], uri_size-sym_after_double_dash, "*:*%s", &uri[sym_at]);
    }
}

// trims a null-terminated str in-place for blanks on both ends
// returns new length
size_t string_trim(char *str)
{
    size_t len = strlen(str);

    // l-trim:
    // find first non-space char, then memmove everything back n chars
    for (size_t i = 0; i < len; i++)
    {
        if (!isspace(str[i]))
        {
            memmove(str, str + i, len - i + 1); // incl null-terminator
            len -= i;
            break;
        }
    }

    // r-trim:
    // find last non-space character, then set +1 = \0
    for (size_t i = len - 1; i < len; i--)
    {
        if (!isspace(str[i]))
        {
            str[i + 1] = '\0';
            len = i + 1;
            break;
        }
    }

    return len;
}
