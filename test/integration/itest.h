#pragma once

#include "app.h"

void setup(char *tmppath);
void teardown(char *tmppath);
bool file_exists_in_path(char *tmppath, char* relpath);
App_Status_Code basic_app_main_run(const char* manifest);
App_Status_Code basic_app_main_run_no_teardown(const char *manifest, char** out_tmppath);
