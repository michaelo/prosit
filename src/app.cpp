#include <cstring>

#include "app.h"

#include "mlib/utils.imp.h"
#include "mlib/defer.imp.h"

void string_trim(char* str) {
    for(int i=strlen(str)-1; i>=0; i--) {
        if(str[i] != ' ') break;
        str[i] = '\0';
    }
}

bool parse_manifest(const char* path, Manifest** manifest_out) {
    size_t file_size;
    char* buf = file_to_buf(path, &file_size);
    defer(delete(buf));

    Manifest* manifest = new Manifest;
    int num_entries = 0;

    buf_pr_token(buf, "\n", [manifest, &num_entries](char* line) {
        if(line[0] == '#') return;

        // TODO: Error handling
        sscanf(line, "%[^:]: %[^>] > %[^\n]", (char*)&manifest->entries[num_entries].type, (char*)&manifest->entries[num_entries].src, (char*)&manifest->entries[num_entries].dst);
        string_trim(manifest->entries[num_entries].type);
        string_trim(manifest->entries[num_entries].src);
        string_trim(manifest->entries[num_entries].dst);

        printf("Parsed: '%s' '%s' -> '%s'\n",
            manifest->entries[num_entries].type,
            manifest->entries[num_entries].src,
            manifest->entries[num_entries].dst);
        num_entries++;
    });
    manifest->length = num_entries;

    *manifest_out = manifest;
    return true;
}

void free(Manifest* m) {
    delete(m);
}

void cmd_update() {

}

int app_main(int argc, char** argv) {
    for(int i=0; i<argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    // Read manifest? Or to be handled by command handlers that need it? For now: lazy, ie. in cmd-handler
    // Parse arguments
    // Establish context
    // Send context + parsed manifest to appropriate command handler (update ... etc)
    return 0;
}

const char* app_version() {
    return "0.1.0"; // TODO: Get from build sys
}
