#pragma once

#include "app.h"

struct Test_Context
{
    char workspace_path[MAX_PATH_LEN];
    char pre_test_cwd[MAX_PATH_LEN];
};

typedef const char* TESTFILEARR[];

// Creates a temp-folder, cwds to it, stores the inital cwd to context
void test_setup(Test_Context *context);

// Deletes the tmp-folder, cwds back to initail cwd
void test_teardown(Test_Context *context);

// Sets up a prosit.manifest in the test-directory with the passed contents, runs app_main() and returns the status
// Assumes test_setup and test_teardown is handled outside
App_Status_Code test_run_with_manifest(Test_Context *context, const char* manifest_contents);

// Does a full setup, run, teardown and returns the status
App_Status_Code test_allinone(const char* manifest_contents);

// As test_allinone, but verifies the existense of files/folders in the tmp-workspace
bool test_succeeds_and_contains_files(const char* manifest_contents, size_t files_rel_path_count, TESTFILEARR files_rel_path);

// As test_run_with_manifest, but verifies the existense of files/folders in the tmp-workspace
bool test_run_with_manifest_and_contains_files(Test_Context *context, const char* manifest_contents, size_t files_rel_path_count, TESTFILEARR files_rel_path);

