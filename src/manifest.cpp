#include <cstdio>
#include <cstring>
#include <cctype>

#include "app.h"

#include "mlib/utils.imp.h"
#include "mlib/defer.imp.h"

void string_trim(char *str)
{
    // l-trim?
    // r-trim:
    for (int i = strlen(str) - 1; i >= 0; i--)
    {
        if (str[i] != ' ')
            break;
        str[i] = '\0';
    }
}

// TODO: Parse directly from buf, to remove dependencies on files for e.g. tests?
// manifest_out must be freed if function returns true
bool manifest_parse(const char *path, Manifest **manifest_out)
{
    size_t file_size;
    char *buf = file_to_buf(path, &file_size);
    if(buf == nullptr) {
        printf("ERROR: Could not open file %s\n", path);
        return false;
    }
    defer(delete (buf));

    Manifest *manifest = new Manifest;
    defer(delete (manifest));
    int num_entries = 0;
    int line_no = 0;
    bool any_errors = false;
    buf_pr_token(buf, "\n", [path, manifest, &any_errors, &num_entries, &line_no](char *line) {
        line_no++;
        size_t line_len = strlen(line);
        
        // Ignore comment-lines
        if (line[0] == '#')
            return;

        // Ignore whitespace-lines
        {
            bool all_space = true;
            for (size_t i = 0; i < line_len; i++)
            {
                if (!isspace(line[i])) {
                    all_space = false;
                    break;
                }
            }

            if(all_space) {
                return;
            }
        }

        // TODO: Error handling (syntax-errors)
        int num_matches = sscanf(line, "%[^:]: %[^>] > %[^\n]", (char *)&manifest->entries[num_entries].type, (char *)&manifest->entries[num_entries].src, (char *)&manifest->entries[num_entries].dst);

        if (num_matches < 3)
        {
            printf("ERROR %s:%d: Could not parse line\n", path, line_no);
            printf("  -> %s\n", line);
            any_errors = true;
            return;
        }

        manifest->entries[num_entries].line_in_manifest = line_no;
        string_trim(manifest->entries[num_entries].type);
        string_trim(manifest->entries[num_entries].src);
        string_trim(manifest->entries[num_entries].dst);

        num_entries++;
    });
    manifest->length = num_entries;

    *manifest_out = new Manifest;
    memcpy(*manifest_out, manifest, sizeof(Manifest));
    return !any_errors;
}

void manifest_free(Manifest *m)
{
    delete (m);
}
