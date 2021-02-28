#include "itest.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <filesystem>

#include "app.h"
#include "mlib/defer.imp.h"
#include "platform.h"

static constexpr int MAX_BASEFOLDER_CHECK_STEPS = 10;
static constexpr char BASEFOLDER_CHECKFILE[] = "meson.build";

namespace fs = std::filesystem;

static bool file_exists_in_path(const char *tmppath, const char* relpath)
{
    char fullpath[MAX_PATH_LEN];
    fullpath[0] = '\0';
    snprintf((char*)fullpath, sizeof(fullpath), "%s/%s", tmppath, relpath);
    std::error_code error_code;
    return fs::exists(fullpath, error_code);
}

static bool get_first_parent_containing(const char *start_path, char *out_buf, size_t out_buf_size, const char *file_to_check_for)
{
    // Iterate parent-by-parent to get the folder with some predefined content
    fs::path current(start_path);

    int i = 0;
    while(fs::exists(current) && i++ < MAX_BASEFOLDER_CHECK_STEPS) {
        if(file_exists_in_path(current.u8string().c_str(), file_to_check_for)) {
            strncpy(out_buf, current.u8string().c_str(), out_buf_size);
            return true;
        }
        current = current.parent_path();
    }
    return false;
}

// Sets up a temporary folder to be used as a test-workspace. Changes cwd to it.
void test_setup(Test_Context *context)
{
    // Create a tmp-folder.
    std::error_code error_code;
    fs::path tmp_dir = fs::temp_directory_path(error_code);

    // Assign semi-unique subfolder name to allow parallell runs. Att! Not particularily robust.
    bool dir_created = false;
    {
        for(int i=0; i<100; i++) {
            snprintf(context->workspace_path, sizeof(context->workspace_path), "%s/%s_%d", tmp_dir.u8string().c_str(), "prosit_inttest", i);
            printf("Attempting to create tmp-folder: %s\n", context->workspace_path);
            if(fs::create_directories(context->workspace_path, error_code)) {
                printf("Succeeded in creating: %s\n", context->workspace_path);
                dir_created = true;
                break;
            }
        }
    }

    assert(dir_created);

    // Setup env-variable to point to test-files
    char base_path[256];

    assert(get_first_parent_containing(fs::current_path().u8string().c_str(), base_path, sizeof(base_path), BASEFOLDER_CHECKFILE));
    char tmppath_raw[MAX_PATH_LEN];
    assert(snprintf(tmppath_raw, sizeof(tmppath_raw), "%s/%s", base_path, "test/integration/testfiles") > 0);
    fs::path testfiles_path = fs::canonical(tmppath_raw);
    setenv("PROSIT_ITEST_TESTFILES", (char *)testfiles_path.u8string().c_str(), 1);

    strncpy(context->pre_test_cwd, fs::current_path().u8string().c_str(), sizeof(context->pre_test_cwd));
    fs::current_path(context->workspace_path);
}

App_Status_Code test_run_with_manifest(Test_Context *context, const char* manifest_contents)
{
    // Write manifest_contents to workspace/prosit.manifest
    {
        char scrap[MAX_PATH_LEN];
        snprintf(scrap, sizeof(scrap), "%s/prosit.manifest", context->workspace_path);
        FILE *fp = fopen(scrap, "wb");
        assert(fp != NULL);
        defer(fclose(fp));

        fwrite(manifest_contents, strlen(manifest_contents), 1, fp);
    }

    // Call app_main() and return result
    // Precondition: cwd is in workspace. 
    const char *argv[] = {
        "prosit",
        "update"};

    return app_main(2, (char**)argv);
}


bool test_run_with_manifest_and_contains_files(Test_Context *context, const char* manifest_contents, size_t files_rel_path_count, TESTFILEARR files_rel_path)
{
    if(test_run_with_manifest(context, manifest_contents) != App_Status_Code::Ok) {
        return false;
    }

    for(size_t i=0; i<files_rel_path_count; i++) {
        if(!file_exists_in_path(context->workspace_path, files_rel_path[i])) {
            fprintf(stderr, "ERROR: %s not found in workspace (%s)\n", files_rel_path[i], context->workspace_path);
            return false;
        }
    }
    return true;
}

void test_teardown(Test_Context *context)
{
    std::error_code error_code;
    // Set cwd back to initial dir
    fs::current_path(context->pre_test_cwd);
    // Delete tmp-folder
    if(fs::remove_all(context->workspace_path, error_code) == -1)
    {
        // TODO: This has been observed for git-related tests under Windows 10. This will eventually cause the need for manual cleanup in temp-folder
        fprintf(stderr, "ERROR: Could not remove temporary files at %s (%s)\n", context->workspace_path, error_code.message().c_str());
    }
}


App_Status_Code test_allinone(const char* manifest_contents) {
    Test_Context context;
    test_setup(&context);
    defer(test_teardown(&context));

    return test_run_with_manifest(&context, manifest_contents);
}

bool test_succeeds_and_contains_files(const char* manifest_contents, size_t files_rel_path_count, TESTFILEARR files_rel_path) {
    Test_Context context;
    test_setup(&context);
    defer(test_teardown(&context));

    if(test_run_with_manifest(&context, manifest_contents) != App_Status_Code::Ok) {
        return false;
    }

    for(size_t i=0; i<files_rel_path_count; i++) {
        if(!file_exists_in_path(context.workspace_path, files_rel_path[i])) {
            fprintf(stderr, "ERROR: %s not found in workspace (%s)\n", files_rel_path[i], context.workspace_path);
            return false;
        }
    }
    return true;
}
