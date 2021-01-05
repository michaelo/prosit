#pragma once

int app_main(int argc, char** argv);
const char* app_version();

struct Context {

};

enum Handler_Status {
    OK,
    Error,
};

// Handlers
Handler_Status handle_git(Context*, char* config);
Handler_Status handle_file(Context*, char* config);


// Manifest
struct Manifest_Entry {
    char type[16];
    char src[256];
    char dst[256];
};

struct Manifest {
    int length = 0;
    Manifest_Entry entries[128]; // TODO: Fix to dynamic size based in input data (two-pass parse)
};

bool parse_manifest(const char* path, Manifest** manifest_out);
void free(Manifest* m);