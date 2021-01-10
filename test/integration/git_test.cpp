#include "app.h"
#include "test.h"
#include "itest.h"

#include "mlib/defer.imp.h"

#include <cassert>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}