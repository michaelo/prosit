#pragma once

#include "app.h"

void setup(char *tmppath);
void teardown(char *tmppath);
App_Status_Code basic_app_main_run(const char* manifest);
