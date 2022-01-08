#include <cstdio>
#include <cstring>
#include <cctype>

#include "app.h"

#include "mlib/utils.imp.h"
#include "mlib/defer.imp.h"

// manifest_out must be freed if function returns true
bool manifest_parse_buf(char *buf, Manifest **manifest_out)
{
    Manifest *manifest = new Manifest;
    defer(delete (manifest));
    int num_entries = 0;
    int line_no = 0;
    bool any_errors = false;
    buf_pr_token(buf, "\r\n", [manifest, &any_errors, &num_entries, &line_no](char *line) {
        line_no++;
        size_t line_len = strlen(line);
        if (line_len == 0)
        {
            return;
        }
        // Ignore comment-lines
        if (line[0] == '#')
        {
            return;
        }

        // Ignore all-space lines
        {
            int start_i = 0;
            bool all_space = true;
            for (size_t i = 0; i < line_len; i++)
            {
                if (!isspace(line[i]))
                {
                    start_i = i;
                    all_space = false;
                    break;
                }
            }

            if (all_space)
            {
                return;
            }

            // Trim away leading space
            line = &line[start_i];
        }

        char *type_sep, *srcdest_sep;
        type_sep = strchr(line, ':');
        if(type_sep) {
            srcdest_sep = strchr(type_sep, '>');
        }

        if (!(type_sep && srcdest_sep))
        {
            fprintf(stderr, "ERROR: Could not parse line %d\n", line_no);
            any_errors = true;
            return;
        }

        // Extract
        *type_sep = '\0';
        *srcdest_sep = '\0';
        strncpy(manifest->entries[num_entries].type, line, sizeof(manifest->entries[num_entries].type));
        strncpy(manifest->entries[num_entries].src, type_sep + 1, sizeof(manifest->entries[num_entries].src));
        strncpy(manifest->entries[num_entries].dst, srcdest_sep + 1, sizeof(manifest->entries[num_entries].dst));

        manifest->entries[num_entries].line_in_manifest = line_no;

        // Cleanup / trim
        string_trim(manifest->entries[num_entries].type);
        string_trim(manifest->entries[num_entries].src);
        string_trim(manifest->entries[num_entries].dst);
        expand_environment_vars(manifest->entries[num_entries].src, sizeof(manifest->entries[num_entries].src));
        expand_environment_vars(manifest->entries[num_entries].dst, sizeof(manifest->entries[num_entries].dst));

        num_entries++;
    });
    manifest->length = num_entries;

    // *manifest_out = manifest;
    if (!any_errors && manifest_out)
    {
        *manifest_out = new Manifest;
        memcpy(*manifest_out, manifest, sizeof(Manifest));
    }

    return !any_errors;
}

// manifest_out must be freed if function returns true
bool manifest_parse(const char *path, Manifest **manifest_out)
{
    size_t file_size;
    char *buf = file_to_buf(path, &file_size);
    if (buf == nullptr)
    {
        printf("ERROR: Could not open file %s\n", path);
        return false;
    }
    defer(delete (buf));

    return manifest_parse_buf(buf, manifest_out);
}

void manifest_free(Manifest *m)
{
    delete (m);
}
